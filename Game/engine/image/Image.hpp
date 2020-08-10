#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include "Descriptor.hpp"

class CommandPool;
class Device;
class ImageView;

class Image
{
protected:
    Image(std::shared_ptr<Device> device, uint32_t binding, VkShaderStageFlagBits stageFlags);
    ~Image();
    
public:
    operator VkImage();
    VkDeviceMemory memory();
    VkImageView imageView();
    uint32_t mipLevels();
    
    const Descriptor& descriptor();
    
    static void transitionImageLayout(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspect, uint32_t mipLevels);
    
protected:
    void createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits msaaSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlagBits flags);
    
protected:
    VkImage _image;
    VkDeviceMemory _imageMemory;
    std::shared_ptr<ImageView> _imageView;
    uint32_t _mipLevels{1};
    uint32_t _arrayLayers{1};
    VkSampleCountFlagBits _msaaSamples{VK_SAMPLE_COUNT_1_BIT};
    
    const Descriptor _descriptor;
    
    std::shared_ptr<Device> _device;
};
