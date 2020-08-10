#pragma once

#include "Image.hpp"

#include <vulkan/vulkan.h>

#include <memory>

class CommandPool;
class Device;

class CubeMapImage : public Image
{
public:
    CubeMapImage(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, VkFormat imageFormat, uint32_t binding, VkShaderStageFlagBits stageFlags);
    ~CubeMapImage();
    
    VkSampler sampler();
        
private:
    void createSampler();
    
private:
    VkSampler _sampler;
};
