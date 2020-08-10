#include "CommandBuffers.hpp"

#include <array>
#include <exception>

#include "CommandPool.hpp"
#include "CubeMapImage.hpp"
#include "Device.hpp"
#include "EfficientBuffer.hpp"
#include "Framebuffer.hpp"
#include "InstanceBufferObject.hpp"
#include "Object.hpp"
#include "OffscreenPass.hpp"
#include "PushConstants.hpp"
#include "Scene.hpp"
#include "ScenePass.hpp"
#include "StencilPass.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CommandBuffers::Private
{
public:
    Private(std::shared_ptr<Device> device,
            std::shared_ptr<CommandPool> commandPool,
            std::shared_ptr<ScenePass> scenePass,
            std::shared_ptr<OffscreenPass> offscreenPass,
            std::shared_ptr<OffscreenPass> environmentMapPass,
            std::shared_ptr<StencilPass> stencilPass,
            std::shared_ptr<Scene> scene,
            std::shared_ptr<CubeMapImage> cubeMapImage,
            std::vector<std::shared_ptr<CubeMapImage>> environmentMapImages)
        : _device(device)
        , _commandPool(commandPool)
    {
        _commandBuffers.resize(scenePass->amountOfFramebuffers());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = *_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) _commandBuffers.size();

        if (vkAllocateCommandBuffers(*_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (size_t i = 0; i < _commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }
            
            // Render shadow map
            {
                VkViewport viewport{};
                viewport.width = 1024.0f;
                viewport.height = 1024.0f;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(_commandBuffers[i], 0, 1, &viewport);
                
                VkRect2D scissor{};
                scissor.extent.width = 1024;
                scissor.extent.height = 1024;
                scissor.offset.x = 0;
                scissor.offset.y = 0;
                vkCmdSetScissor(_commandBuffers[i], 0, 1, &scissor);

                VkClearValue clearValues[2];
                clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
                clearValues[1].depthStencil = { 1.0f, 0 };
                
                for (uint32_t face = 0; face < 6; face++) {
                    updateCubeFace(offscreenPass, scene, cubeMapImage, clearValues, face, 0, i, false);
                }
            }
            
            // Render environment map (every 60 frames)
            static int environmentRenderIteration = 0;
            if (environmentRenderIteration % 60 == 0) {
                for (uint32_t j = 0; j < 4; ++j) {
                    VkViewport viewport{};
                    viewport.width = 1024.0f;
                    viewport.height = 1024.0f;
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    vkCmdSetViewport(_commandBuffers[i], 0, 1, &viewport);
                    
                    VkRect2D scissor{};
                    scissor.extent.width = 1024;
                    scissor.extent.height = 1024;
                    scissor.offset.x = 0;
                    scissor.offset.y = 0;
                    vkCmdSetScissor(_commandBuffers[i], 0, 1, &scissor);

                    VkClearValue clearValues[2];
                    clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
                    clearValues[1].depthStencil = { 1.0f, 0 };

                    for (uint32_t face = 0; face < 6; face++) {
                        updateCubeFace(environmentMapPass, scene, environmentMapImages[j], clearValues, face, j+1, i, true);
                    }
                }
            }
            environmentRenderIteration++;
            
            // Render scene with shadows
            {
                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = *scenePass->renderPass();
                renderPassInfo.framebuffer = *scenePass->framebuffers(i);
                renderPassInfo.renderArea.offset = {0, 0};
                renderPassInfo.renderArea.extent = scenePass->extent();
                
                std::array<VkClearValue, 2> clearValues{};
                clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
                clearValues[1].depthStencil = {1.0f, 0};
                
                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    VkViewport viewport{};
                    viewport.width = scenePass->extent().width;
                    viewport.height = scenePass->extent().height;
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    vkCmdSetViewport(_commandBuffers[i], 0, 1, &viewport);
                    
                    VkRect2D scissor{};
                    scissor.extent.width = scenePass->extent().width;
                    scissor.extent.height = scenePass->extent().height;
                    scissor.offset.x = 0;
                    scissor.offset.y = 0;
                    vkCmdSetScissor(_commandBuffers[i], 0, 1, &scissor);

                    vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *scenePass->pipelineLayout(), 0, 1, &scenePass->descriptorSet(i), 0, nullptr);

                
                    vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *scenePass->pipeline());

                    VkDeviceSize offsets[] = {0};
                    for (auto&& object : scene->objects()) {
                        VkBuffer vertexBuffers[] = {*object->vertexBuffer()};
                        VkBuffer instanceBuffers[] = {*object->instanceBufferObject()->buffer(i)};
                        vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
                        vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
                        vkCmdBindIndexBuffer(_commandBuffers[i], *object->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                        vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(object->indexBuffer()->amount()), object->amountOfInstances(), 0, 0, 0);
                    }

                    {
                        VkBuffer vertexBuffers[] = {*scene->draughts()->boardObject()->vertexBuffer()};
                        VkBuffer instanceBuffers[] = {*scene->draughts()->boardObject()->instanceBufferObject()->buffer(i)};
                        vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
                        vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
                        vkCmdBindIndexBuffer(_commandBuffers[i], *scene->draughts()->boardObject()->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                        vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(scene->draughts()->boardObject()->indexBuffer()->amount()), scene->draughts()->boardObject()->amountOfInstances(), 0, 0, 0);
                    }
                
                    {
                        VkBuffer vertexBuffers[] = {*scene->draughts()->draughtsObject()->vertexBuffer()};
                        VkBuffer instanceBuffers[] = {*scene->draughts()->draughtsObject()->instanceBufferObject()->buffer(i)};
                        vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
                        vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
                        vkCmdBindIndexBuffer(_commandBuffers[i], *scene->draughts()->draughtsObject()->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                        vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(scene->draughts()->draughtsObject()->indexBuffer()->amount()), scene->draughts()->draughtsObject()->amountOfInstances(), 0, 0, 0);
                    }
                    
                    {
                        VkBuffer vertexBuffers[] = {*scene->sphere()->vertexBuffer()};
                        VkBuffer instanceBuffers[] = {*scene->sphere()->instanceBufferObject()->buffer(i)};
                        vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
                        vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
                        vkCmdBindIndexBuffer(_commandBuffers[i], *scene->sphere()->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                        vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(scene->sphere()->indexBuffer()->amount()), scene->sphere()->amountOfInstances(), 0, 0, 0);
                    }
                
                vkCmdEndRenderPass(_commandBuffers[i]);
            }
            
            // Render scene for stencil
            {
                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = *stencilPass->renderPass();
                renderPassInfo.framebuffer = *stencilPass->framebuffers(i);
                renderPassInfo.renderArea.offset = {0, 0};
                renderPassInfo.renderArea.extent = stencilPass->extent();
                
                std::array<VkClearValue, 2> clearValues{};
                clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
                clearValues[1].depthStencil = {1.0f, 0};
                
                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    VkViewport viewport{};
                    viewport.width = stencilPass->extent().width;
                    viewport.height = stencilPass->extent().height;
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    vkCmdSetViewport(_commandBuffers[i], 0, 1, &viewport);
                    
                    VkRect2D scissor{};
                    scissor.extent.width = stencilPass->extent().width;
                    scissor.extent.height = stencilPass->extent().height;
                    scissor.offset.x = 0;
                    scissor.offset.y = 0;
                    vkCmdSetScissor(_commandBuffers[i], 0, 1, &scissor);

                    vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *stencilPass->pipelineLayout(), 0, 1, &stencilPass->descriptorSet(i), 0, nullptr);
                
                    vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *stencilPass->pipeline());

                    VkDeviceSize offsets[] = {0};
                    for (auto&& object : scene->objects()) {
                        VkBuffer vertexBuffers[] = {*object->vertexBuffer()};
                        VkBuffer instanceBuffers[] = {*object->instanceBufferObject()->buffer(i)};
                        vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
                        vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
                        vkCmdBindIndexBuffer(_commandBuffers[i], *object->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                        vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(object->indexBuffer()->amount()), object->amountOfInstances(), 0, 0, 0);
                    }

                    {
                        VkBuffer vertexBuffers[] = {*scene->draughts()->boardObject()->vertexBuffer()};
                        VkBuffer instanceBuffers[] = {*scene->draughts()->boardObject()->instanceBufferObject()->buffer(i)};
                        vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
                        vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
                        vkCmdBindIndexBuffer(_commandBuffers[i], *scene->draughts()->boardObject()->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                        vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(scene->draughts()->boardObject()->indexBuffer()->amount()), scene->draughts()->boardObject()->amountOfInstances(), 0, 0, 0);
                    }
                
                    {
                        VkBuffer vertexBuffers[] = {*scene->draughts()->draughtsObject()->vertexBuffer()};
                        VkBuffer instanceBuffers[] = {*scene->draughts()->draughtsObject()->instanceBufferObject()->buffer(i)};
                        vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
                        vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
                        vkCmdBindIndexBuffer(_commandBuffers[i], *scene->draughts()->draughtsObject()->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                        vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(scene->draughts()->draughtsObject()->indexBuffer()->amount()), scene->draughts()->draughtsObject()->amountOfInstances(), 0, 0, 0);
                    }
                
                vkCmdEndRenderPass(_commandBuffers[i]);
            }

            if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }
    
    ~Private()
    {
        vkFreeCommandBuffers(*_device, *_commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
    }
    
    VkCommandBuffer& get(size_t index)
    {
        return _commandBuffers.at(index);
    }
    
private:
    void updateCubeFace(std::shared_ptr<OffscreenPass> offscreenPass, std::shared_ptr<Scene> scene, std::shared_ptr<CubeMapImage> cubeMapImage, VkClearValue clearValues[2], uint32_t faceIndex, uint32_t referenceIndex, size_t i, bool environmentMap)
    {
        VkRenderPassBeginInfo renderPassBeginInfo {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = *offscreenPass->renderPass();
        renderPassBeginInfo.framebuffer = *offscreenPass->framebuffers(i);
        renderPassBeginInfo.renderArea.extent = offscreenPass->extent();
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;

        // Update view matrix via push constant
        PushConstants pushConstants;
        pushConstants.proj = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 1024.0f);
        pushConstants.referencePointIndex = referenceIndex;
        pushConstants.view = glm::mat4(1.0f);
        switch (faceIndex)
        {
        case 0: // POSITIVE_X
            pushConstants.view = glm::rotate(pushConstants.view, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            pushConstants.view = glm::rotate(pushConstants.view, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case 1:    // NEGATIVE_X
            pushConstants.view = glm::rotate(pushConstants.view, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            pushConstants.view = glm::rotate(pushConstants.view, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case 2:    // POSITIVE_Y
            pushConstants.view = glm::rotate(pushConstants.view, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case 3:    // NEGATIVE_Y
            pushConstants.view = glm::rotate(pushConstants.view, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case 4:    // POSITIVE_Z
            pushConstants.view = glm::rotate(pushConstants.view, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case 5:    // NEGATIVE_Z
            pushConstants.view = glm::rotate(pushConstants.view, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        }

        // Render scene from cube face's point of view
        vkCmdBeginRenderPass(_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Update shader push constant block
        // Contains current face view matrix
        vkCmdPushConstants(
            _commandBuffers[i],
            *offscreenPass->pipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(PushConstants),
            &pushConstants);

        vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *offscreenPass->pipeline());
        vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *offscreenPass->pipelineLayout(), 0, 1, &offscreenPass->descriptorSet(i), 0, NULL);

        VkDeviceSize offsets[1] = { 0 };
        for (auto&& object : scene->objects()) {
            VkBuffer vertexBuffers[] = {*object->vertexBuffer()};
            VkBuffer instanceBuffers[] = {*object->instanceBufferObject()->buffer(i)};
            vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
            vkCmdBindIndexBuffer(_commandBuffers[i], *object->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(object->indexBuffer()->amount()), object->amountOfInstances(), 0, 0, 0);
        }

        {
            VkBuffer vertexBuffers[] = {*scene->draughts()->boardObject()->vertexBuffer()};
            VkBuffer instanceBuffers[] = {*scene->draughts()->boardObject()->instanceBufferObject()->buffer(i)};
            vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
            vkCmdBindIndexBuffer(_commandBuffers[i], *scene->draughts()->boardObject()->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(scene->draughts()->boardObject()->indexBuffer()->amount()), scene->draughts()->boardObject()->amountOfInstances(), 0, 0, 0);
        }
    
        {
            VkBuffer vertexBuffers[] = {*scene->draughts()->draughtsObject()->vertexBuffer()};
            VkBuffer instanceBuffers[] = {*scene->draughts()->draughtsObject()->instanceBufferObject()->buffer(i)};
            vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
            vkCmdBindIndexBuffer(_commandBuffers[i], *scene->draughts()->draughtsObject()->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(scene->draughts()->draughtsObject()->indexBuffer()->amount()), scene->draughts()->draughtsObject()->amountOfInstances(), 0, 0, 0);
        }
        
        auto instanceToSkip = environmentMap ? pushConstants.referencePointIndex-1 : -1;
        for (int k = 0; k < 4; ++k) {
            if (k == instanceToSkip) {
                continue;
            }
            
            VkBuffer vertexBuffers[] = {*scene->sphere()->vertexBuffer()};
            VkBuffer instanceBuffers[] = {*scene->sphere()->instanceBufferObject()->buffer(i)};
            vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindVertexBuffers(_commandBuffers[i], 1, 1, instanceBuffers, offsets);
            vkCmdBindIndexBuffer(_commandBuffers[i], *scene->sphere()->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(scene->sphere()->indexBuffer()->amount()), 1, 0, 0, k);
        }
        
        vkCmdEndRenderPass(_commandBuffers[i]);
        
        // Make sure color writes to the framebuffer are finished before using it as transfer source
        setImageLayout(
            _commandBuffers[i],
            *offscreenPass->colorImage(),
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        VkImageSubresourceRange cubeFaceSubresourceRange = {};
        cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cubeFaceSubresourceRange.baseMipLevel = 0;
        cubeFaceSubresourceRange.levelCount = 1;
        cubeFaceSubresourceRange.baseArrayLayer = faceIndex;
        cubeFaceSubresourceRange.layerCount = 1;

        // Change image layout of one cubemap face to transfer destination
        setImageLayout(
            _commandBuffers[i],
            *cubeMapImage,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            cubeFaceSubresourceRange);

        // Copy region for transfer from framebuffer to cube face
        VkImageCopy copyRegion = {};

        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.srcOffset = { 0, 0, 0 };

        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.baseArrayLayer = faceIndex;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.dstOffset = { 0, 0, 0 };

        copyRegion.extent.width = 1024;
        copyRegion.extent.height = 1024;
        copyRegion.extent.depth = 1;

        // Put image copy into command buffer
        vkCmdCopyImage(
            _commandBuffers[i],
            *offscreenPass->colorImage(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            *cubeMapImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copyRegion);

        // Transform framebuffer color attachment back
        setImageLayout(
            _commandBuffers[i],
            *offscreenPass->colorImage(),
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // Change image layout of copied face to shader read
        setImageLayout(
            _commandBuffers[i],
            *cubeMapImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            cubeFaceSubresourceRange);
    }
    
    void setImageLayout(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
    {
        // Create an image barrier object
        VkImageMemoryBarrier imageMemoryBarrier {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (oldImageLayout)
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            // Image layout is undefined (or does not matter)
            // Only valid as initial layout
            // No flags required, listed only for completeness
            imageMemoryBarrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            // Image is preinitialized
            // Only valid as initial layout for linear images, preserves memory contents
            // Make sure host writes have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image is a color attachment
            // Make sure any writes to the color buffer have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image is a depth/stencil attachment
            // Make sure any writes to the depth/stencil buffer have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image is a transfer source
            // Make sure any reads from the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image is a transfer destination
            // Make sure any writes to the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image is read by a shader
            // Make sure any shader reads from the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
        switch (newImageLayout)
        {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image will be used as a transfer destination
            // Make sure any writes to the image have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image will be used as a transfer source
            // Make sure any reads from the image have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image will be used as a color attachment
            // Make sure any writes to the color buffer have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image layout will be used as a depth/stencil attachment
            // Make sure any writes to depth/stencil buffer have been finished
            imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image will be read in a shader (sampler, input attachment)
            // Make sure any writes to the image have been finished
            if (imageMemoryBarrier.srcAccessMask == 0)
            {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
        }

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(
            cmdbuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
    }

    // Fixed sub resource on first mip level and layer
    void setImageLayout(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
    {
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = aspectMask;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;
        setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
    }
    
private:
    std::vector<VkCommandBuffer> _commandBuffers;
    
    std::shared_ptr<Device> _device;
    std::shared_ptr<CommandPool> _commandPool;
};

CommandBuffers::CommandBuffers(std::shared_ptr<Device> device,
                               std::shared_ptr<CommandPool> commandPool,
                               std::shared_ptr<ScenePass> scenePass,
                               std::shared_ptr<OffscreenPass> offscreenPass,
                               std::shared_ptr<OffscreenPass> environmentMapPass,
                               std::shared_ptr<StencilPass> stencilPass,
                               std::shared_ptr<Scene> scene,
                               std::shared_ptr<CubeMapImage> cubeMapImage,
                               std::vector<std::shared_ptr<CubeMapImage>> environmentMapImages)
    : _delegate(std::make_unique<Private>(device, commandPool, scenePass, offscreenPass, environmentMapPass, stencilPass, scene, cubeMapImage, environmentMapImages))
{
    
}

CommandBuffers::~CommandBuffers()
{
    
}

VkCommandBuffer& CommandBuffers::get(size_t index)
{
    return _delegate->get(index);
}
