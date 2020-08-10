#pragma once

#include "Image.hpp"

#include <vulkan/vulkan.h>

#include <memory>

class Device;

class SwapChainImage
{
public:
    SwapChainImage(std::shared_ptr<Device> device, VkImage image, VkFormat imageFormat);
    ~SwapChainImage();
    
    VkImageView imageView();
    
private:
    std::shared_ptr<ImageView> _imageView;
};
