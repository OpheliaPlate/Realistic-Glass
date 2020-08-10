#pragma once

#include "Image.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <string>

class CommandPool;
class Device;
class Sampler;

class TextureImage : public Image
{
public:
    TextureImage(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, const std::string path, uint32_t binding, VkShaderStageFlagBits stageFlags);
    ~TextureImage();
    
    VkSampler sampler();
    
private:
    void copyBufferToImage(std::shared_ptr<CommandPool> commandPool, VkBuffer buffer, uint32_t width, uint32_t height);
    void generateMipmaps(std::shared_ptr<CommandPool> commandPool, VkFormat imageFormat, int32_t texWidth, int32_t texHeight);
    void createSampler();
    
private:
    VkSampler _sampler;
};
