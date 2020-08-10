#pragma once

#include "Image.hpp"

#include <vulkan/vulkan.h>

#include <memory>

class CommandPool;
class Device;

class DepthImage : public Image
{
public:
    DepthImage(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, VkExtent2D extent, bool withMsaa);
    ~DepthImage();
    
public:
    static VkFormat findDepthFormat(std::shared_ptr<Device> device);
    static VkFormat findSupportedFormat(std::shared_ptr<Device> device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
};
