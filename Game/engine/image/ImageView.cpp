#include "ImageView.hpp"

#include <exception>

#include "Buffer.hpp"
#include "Device.hpp"
#include "OneTimeCommandBuffer.hpp"

class ImageView::Private
{
public:
    Private(std::shared_ptr<Device> device, VkImage image, VkFormat format, VkImageViewType viewType, uint32_t layerCount, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
        : _device(device)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = viewType;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = layerCount;

        if (vkCreateImageView(*device, &viewInfo, nullptr, &_imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }
    
    ~Private()
    {
        vkDestroyImageView(*_device, _imageView, nullptr);
    }
    
    operator VkImageView()
    {
        return _imageView;
    }

private:
    VkImageView _imageView;
    
    std::shared_ptr<Device> _device;
};

ImageView::ImageView(std::shared_ptr<Device> device, VkImage image, VkFormat format, VkImageViewType viewType, uint32_t layerCount, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
    : _delegate(std::make_unique<Private>(device, image, format, viewType, layerCount, aspectFlags, mipLevels))
{
    
}

ImageView::~ImageView()
{
    
}

ImageView::operator VkImageView()
{
    return *_delegate;
}
