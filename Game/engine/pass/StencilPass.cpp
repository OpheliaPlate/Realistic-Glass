#include "StencilPass.hpp"

#include "ColorImage.hpp"
#include "DepthImage.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "Framebuffer.hpp"
#include "LightingBufferObject.hpp"
#include "StencilRenderPass.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "PushConstants.hpp"
#include "SwapChainImage.hpp"
#include "UniformBufferObject.hpp"

StencilPass::StencilPass(std::shared_ptr<Device> device,
                         std::shared_ptr<CommandPool> commandPool,
                         std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
                         std::shared_ptr<DescriptorPool> descriptorPool,
                         VkFormat imageFormat,
                         VkExtent2D extent,
                         const std::string vertShader,
                         const std::string fragShader,
                         std::vector<std::shared_ptr<SwapChainImage>> swapChainImages,
                         std::shared_ptr<UniformBufferObject> uniformBufferObject)
    : Pass(device, extent)
{
    _renderPass = std::make_shared<StencilRenderPass>(device, imageFormat);
    
    std::vector<VkPushConstantRange> pushConstantRanges;
    
    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);
    pushConstantRanges.emplace_back(pushConstantRange);
    
    _pipelineLayout = std::make_shared<PipelineLayout>(device, descriptorSetLayout, pushConstantRanges);
    
    _pipeline = std::make_shared<Pipeline>(device, _renderPass, _pipelineLayout, extent, vertShader, fragShader, false, false, VK_CULL_MODE_FRONT_AND_BACK);
    
    _colorImage = std::make_shared<ColorImage>(device, commandPool, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, imageFormat, extent, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false, true);
    _depthImage = std::make_shared<DepthImage>(device, commandPool, extent, false);
    
    _stencilImage = std::make_shared<ColorImage>(device, commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_FORMAT_R8G8B8A8_UNORM, extent, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false, false);
    
    createFramebuffers(swapChainImages);
    createDescriptorSets(descriptorSetLayout, descriptorPool, swapChainImages, uniformBufferObject);
}

StencilPass::~StencilPass()
{
    
}

std::shared_ptr<ColorImage> StencilPass::stencilImage()
{
    return _stencilImage;
}
    
void StencilPass::createFramebuffers(std::vector<std::shared_ptr<SwapChainImage>> swapChainImages)
{
    _framebuffers.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        std::vector<VkImageView> attachments = {
            _colorImage->imageView(),
            _depthImage->imageView(),
        };
        
        _framebuffers[i] = std::make_shared<Framebuffer>(_device, _renderPass, attachments, _extent);
    }
}

void StencilPass::createDescriptorSets(std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
                                       std::shared_ptr<DescriptorPool> descriptorPool,
                                       std::vector<std::shared_ptr<SwapChainImage>> swapChainImages,
                                       std::shared_ptr<UniformBufferObject> uniformBufferObject)
{
    std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), *descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = *descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(swapChainImages.size());
    if (vkAllocateDescriptorSets(*_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = *uniformBufferObject->buffer(i);
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject::Structure);
        
        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
        
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(*_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}
