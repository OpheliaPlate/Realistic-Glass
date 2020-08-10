#pragma once

#include "Image.hpp"

#include <vulkan/vulkan.h>

#include <memory>

class Device;

class ColorImage : public Image
{
public:
    ColorImage(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, VkImageLayout layout, VkFormat imageFormat, VkExtent2D extent, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool withMsaa, bool createImageView);
    ~ColorImage();
};
