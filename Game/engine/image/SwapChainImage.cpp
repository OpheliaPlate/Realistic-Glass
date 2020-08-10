#include "SwapChainImage.hpp"

#include <exception>

#include "Device.hpp"
#include "ImageView.hpp"

SwapChainImage::SwapChainImage(std::shared_ptr<Device> device, VkImage image, VkFormat imageFormat)
{
    _imageView = std::make_shared<ImageView>(device, image, imageFormat, VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

SwapChainImage::~SwapChainImage()
{
    
}

VkImageView SwapChainImage::imageView()
{
    return *_imageView;
}
