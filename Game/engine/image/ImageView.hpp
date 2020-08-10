#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class Device;

class ImageView
{
public:
    ImageView(std::shared_ptr<Device> device, VkImage image, VkFormat format, VkImageViewType viewType, uint32_t layerCount, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    ~ImageView();
    
    operator VkImageView();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
