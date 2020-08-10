#include "ScenePass.hpp"

#include "ColorImage.hpp"
#include "CubeMapImage.hpp"
#include "DepthImage.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "Framebuffer.hpp"
#include "InstanceBufferObject.hpp"
#include "LightingBufferObject.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "PushConstants.hpp"
#include "SwapChainImage.hpp"
#include "SwapChainRenderPass.hpp"
#include "TextureImage.hpp"
#include "UniformBufferObject.hpp"

ScenePass::ScenePass(std::shared_ptr<Device> device,
                     std::shared_ptr<CommandPool> commandPool,
                     std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
                     std::shared_ptr<DescriptorPool> descriptorPool,
                     VkFormat imageFormat,
                     VkExtent2D extent,
                     const std::string vertShader,
                     const std::string fragShader,
                     std::vector<std::shared_ptr<SwapChainImage>> swapChainImages,
                     std::shared_ptr<UniformBufferObject> uniformBufferObject,
                     std::shared_ptr<LightingBufferObject> lightingBufferObject,
                     std::shared_ptr<CubeMapImage> cubeMapImage,
                     std::vector<std::shared_ptr<CubeMapImage>> environmentMapImages,
                     std::vector<std::shared_ptr<TextureImage>> textureImages)
    : Pass(device, extent)
{
    _renderPass = std::make_shared<SwapChainRenderPass>(device, imageFormat);
    
    std::vector<VkPushConstantRange> pushConstantRanges;
    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);
    pushConstantRanges.emplace_back(pushConstantRange);
    
    _pipelineLayout = std::make_shared<PipelineLayout>(device, descriptorSetLayout, pushConstantRanges);
    
    _pipeline = std::make_shared<Pipeline>(device, _renderPass, _pipelineLayout, extent, vertShader, fragShader, false, true, VK_CULL_MODE_BACK_BIT);
    
    _colorImage = std::make_shared<ColorImage>(device, commandPool, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, imageFormat, extent, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, true);
    _depthImage = std::make_shared<DepthImage>(device, commandPool, extent, true);
    
    createFramebuffers(swapChainImages);
    createDescriptorSets(descriptorSetLayout, descriptorPool, swapChainImages, uniformBufferObject, lightingBufferObject, cubeMapImage, environmentMapImages, textureImages);
}

ScenePass::~ScenePass()
{
    
}

void ScenePass::createFramebuffers(std::vector<std::shared_ptr<SwapChainImage>> swapChainImages)
{
    _framebuffers.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        std::vector<VkImageView> attachments = {
            _colorImage->imageView(),
            _depthImage->imageView(),
            swapChainImages[i]->imageView(),
        };
        
        _framebuffers[i] = std::make_shared<Framebuffer>(_device, _renderPass, attachments, _extent);
    }
}

void ScenePass::createDescriptorSets(std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
                                     std::shared_ptr<DescriptorPool> descriptorPool,
                                     std::vector<std::shared_ptr<SwapChainImage>> swapChainImages,
                                     std::shared_ptr<UniformBufferObject> uniformBufferObject,
                                     std::shared_ptr<LightingBufferObject> lightingBufferObject,
                                     std::shared_ptr<CubeMapImage> cubeMapImage,
                                     std::vector<std::shared_ptr<CubeMapImage>> environmentMapImages,
                                     std::vector<std::shared_ptr<TextureImage>> textureImages)
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
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
    
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = *uniformBufferObject->buffer(i);
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject::Structure);
        
        VkWriteDescriptorSet bufferDescriptorWrite;
        bufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        bufferDescriptorWrite.dstSet = _descriptorSets[i];
        bufferDescriptorWrite.dstBinding = 0;
        bufferDescriptorWrite.dstArrayElement = 0;
        bufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bufferDescriptorWrite.descriptorCount = 1;
        bufferDescriptorWrite.pBufferInfo = &bufferInfo;
        bufferDescriptorWrite.pNext = nullptr;
        descriptorWrites.emplace_back(bufferDescriptorWrite);

        VkDescriptorBufferInfo lightingInfo{};
        lightingInfo.buffer = *lightingBufferObject->buffer(i);
        lightingInfo.offset = 0;
        lightingInfo.range = sizeof(LightingBufferObject::Structure);
        
        VkWriteDescriptorSet lightingDescriptorWrite;
        lightingDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        lightingDescriptorWrite.dstSet = _descriptorSets[i];
        lightingDescriptorWrite.dstBinding = 1;
        lightingDescriptorWrite.dstArrayElement = 0;
        lightingDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        lightingDescriptorWrite.descriptorCount = 1;
        lightingDescriptorWrite.pBufferInfo = &lightingInfo;
        lightingDescriptorWrite.pNext = nullptr;
        descriptorWrites.emplace_back(lightingDescriptorWrite);
    
        VkDescriptorImageInfo cubeMapInfo{};
        cubeMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cubeMapInfo.imageView = cubeMapImage->imageView();
        cubeMapInfo.sampler = cubeMapImage->sampler();
        
        VkWriteDescriptorSet cubeMapDescriptorWrite;
        cubeMapDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        cubeMapDescriptorWrite.dstSet = _descriptorSets[i];
        cubeMapDescriptorWrite.dstBinding = 2;
        cubeMapDescriptorWrite.dstArrayElement = 0;
        cubeMapDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cubeMapDescriptorWrite.descriptorCount = 1;
        cubeMapDescriptorWrite.pImageInfo = &cubeMapInfo;
        cubeMapDescriptorWrite.pNext = nullptr;
        descriptorWrites.emplace_back(cubeMapDescriptorWrite);
    
        VkDescriptorImageInfo imageInfo1{};
        imageInfo1.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo1.imageView = textureImages[0]->imageView();
        imageInfo1.sampler = textureImages[0]->sampler();
        
        VkWriteDescriptorSet imageDescriptorWrite1;
        imageDescriptorWrite1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageDescriptorWrite1.dstSet = _descriptorSets[i];
        imageDescriptorWrite1.dstBinding = 3;
        imageDescriptorWrite1.dstArrayElement = 0;
        imageDescriptorWrite1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageDescriptorWrite1.descriptorCount = 1;
        imageDescriptorWrite1.pImageInfo = &imageInfo1;
        imageDescriptorWrite1.pNext = nullptr;
        descriptorWrites.emplace_back(imageDescriptorWrite1);
    
        VkDescriptorImageInfo imageInfo2{};
        imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo2.imageView = textureImages[1]->imageView();
        imageInfo2.sampler = textureImages[1]->sampler();
        
        VkWriteDescriptorSet imageDescriptorWrite2;
        imageDescriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageDescriptorWrite2.dstSet = _descriptorSets[i];
        imageDescriptorWrite2.dstBinding = 4;
        imageDescriptorWrite2.dstArrayElement = 0;
        imageDescriptorWrite2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageDescriptorWrite2.descriptorCount = 1;
        imageDescriptorWrite2.pImageInfo = &imageInfo2;
        imageDescriptorWrite2.pNext = nullptr;
        descriptorWrites.emplace_back(imageDescriptorWrite2);
    
        VkDescriptorImageInfo imageInfo3{};
        imageInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo3.imageView = textureImages[2]->imageView();
        imageInfo3.sampler = textureImages[2]->sampler();
        
        VkWriteDescriptorSet imageDescriptorWrite3;
        imageDescriptorWrite3.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageDescriptorWrite3.dstSet = _descriptorSets[i];
        imageDescriptorWrite3.dstBinding = 5;
        imageDescriptorWrite3.dstArrayElement = 0;
        imageDescriptorWrite3.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageDescriptorWrite3.descriptorCount = 1;
        imageDescriptorWrite3.pImageInfo = &imageInfo3;
        imageDescriptorWrite3.pNext = nullptr;
        descriptorWrites.emplace_back(imageDescriptorWrite3);
    
        VkDescriptorImageInfo imageInfo4{};
        imageInfo4.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo4.imageView = textureImages[3]->imageView();
        imageInfo4.sampler = textureImages[3]->sampler();
        
        VkWriteDescriptorSet imageDescriptorWrite4;
        imageDescriptorWrite4.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageDescriptorWrite4.dstSet = _descriptorSets[i];
        imageDescriptorWrite4.dstBinding = 6;
        imageDescriptorWrite4.dstArrayElement = 0;
        imageDescriptorWrite4.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageDescriptorWrite4.descriptorCount = 1;
        imageDescriptorWrite4.pImageInfo = &imageInfo4;
        imageDescriptorWrite4.pNext = nullptr;
        descriptorWrites.emplace_back(imageDescriptorWrite4);
    
        VkDescriptorImageInfo imageInfo5{};
        imageInfo5.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo5.imageView = textureImages[4]->imageView();
        imageInfo5.sampler = textureImages[4]->sampler();
        
        VkWriteDescriptorSet imageDescriptorWrite5;
        imageDescriptorWrite5.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageDescriptorWrite5.dstSet = _descriptorSets[i];
        imageDescriptorWrite5.dstBinding = 7;
        imageDescriptorWrite5.dstArrayElement = 0;
        imageDescriptorWrite5.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageDescriptorWrite5.descriptorCount = 1;
        imageDescriptorWrite5.pImageInfo = &imageInfo5;
        imageDescriptorWrite5.pNext = nullptr;
        descriptorWrites.emplace_back(imageDescriptorWrite5);

        VkDescriptorImageInfo imageInfo6{};
        imageInfo6.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo6.imageView = textureImages[5]->imageView();
        imageInfo6.sampler = textureImages[5]->sampler();
        
        VkWriteDescriptorSet imageDescriptorWrite6;
        imageDescriptorWrite6.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageDescriptorWrite6.dstSet = _descriptorSets[i];
        imageDescriptorWrite6.dstBinding = 8;
        imageDescriptorWrite6.dstArrayElement = 0;
        imageDescriptorWrite6.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageDescriptorWrite6.descriptorCount = 1;
        imageDescriptorWrite6.pImageInfo = &imageInfo6;
        imageDescriptorWrite6.pNext = nullptr;
        descriptorWrites.emplace_back(imageDescriptorWrite6);
        
        VkDescriptorImageInfo environmentMapInfo1{};
        environmentMapInfo1.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        environmentMapInfo1.imageView = environmentMapImages[0]->imageView();
        environmentMapInfo1.sampler = environmentMapImages[0]->sampler();
        
        VkWriteDescriptorSet environmentMapDescriptorWrite1;
        environmentMapDescriptorWrite1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        environmentMapDescriptorWrite1.dstSet = _descriptorSets[i];
        environmentMapDescriptorWrite1.dstBinding = 9;
        environmentMapDescriptorWrite1.dstArrayElement = 0;
        environmentMapDescriptorWrite1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        environmentMapDescriptorWrite1.descriptorCount = 1;
        environmentMapDescriptorWrite1.pImageInfo = &environmentMapInfo1;
        environmentMapDescriptorWrite1.pNext = nullptr;
        descriptorWrites.emplace_back(environmentMapDescriptorWrite1);
        
        VkDescriptorImageInfo environmentMapInfo2{};
        environmentMapInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        environmentMapInfo2.imageView = environmentMapImages[1]->imageView();
        environmentMapInfo2.sampler = environmentMapImages[1]->sampler();
        
        VkWriteDescriptorSet environmentMapDescriptorWrite2;
        environmentMapDescriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        environmentMapDescriptorWrite2.dstSet = _descriptorSets[i];
        environmentMapDescriptorWrite2.dstBinding = 10;
        environmentMapDescriptorWrite2.dstArrayElement = 0;
        environmentMapDescriptorWrite2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        environmentMapDescriptorWrite2.descriptorCount = 1;
        environmentMapDescriptorWrite2.pImageInfo = &environmentMapInfo2;
        environmentMapDescriptorWrite2.pNext = nullptr;
        descriptorWrites.emplace_back(environmentMapDescriptorWrite2);
        
        VkDescriptorImageInfo environmentMapInfo3{};
        environmentMapInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        environmentMapInfo3.imageView = environmentMapImages[2]->imageView();
        environmentMapInfo3.sampler = environmentMapImages[2]->sampler();
        
        VkWriteDescriptorSet environmentMapDescriptorWrite3;
        environmentMapDescriptorWrite3.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        environmentMapDescriptorWrite3.dstSet = _descriptorSets[i];
        environmentMapDescriptorWrite3.dstBinding = 11;
        environmentMapDescriptorWrite3.dstArrayElement = 0;
        environmentMapDescriptorWrite3.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        environmentMapDescriptorWrite3.descriptorCount = 1;
        environmentMapDescriptorWrite3.pImageInfo = &environmentMapInfo3;
        environmentMapDescriptorWrite3.pNext = nullptr;
        descriptorWrites.emplace_back(environmentMapDescriptorWrite3);
        
        VkDescriptorImageInfo environmentMapInfo4{};
        environmentMapInfo4.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        environmentMapInfo4.imageView = environmentMapImages[3]->imageView();
        environmentMapInfo4.sampler = environmentMapImages[3]->sampler();
        
        VkWriteDescriptorSet environmentMapDescriptorWrite4;
        environmentMapDescriptorWrite4.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        environmentMapDescriptorWrite4.dstSet = _descriptorSets[i];
        environmentMapDescriptorWrite4.dstBinding = 12;
        environmentMapDescriptorWrite4.dstArrayElement = 0;
        environmentMapDescriptorWrite4.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        environmentMapDescriptorWrite4.descriptorCount = 1;
        environmentMapDescriptorWrite4.pImageInfo = &environmentMapInfo4;
        environmentMapDescriptorWrite4.pNext = nullptr;
        descriptorWrites.emplace_back(environmentMapDescriptorWrite4);
        
        vkUpdateDescriptorSets(*_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}
