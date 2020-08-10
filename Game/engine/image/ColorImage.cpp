#include "ColorImage.hpp"

#include "Device.hpp"
#include "ImageView.hpp"

ColorImage::ColorImage(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, VkImageLayout layout, VkFormat imageFormat, VkExtent2D extent, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool withMsaa, bool createImageView)
    : Image(device, 0, VK_SHADER_STAGE_FRAGMENT_BIT)
{
    createImage(extent.width, extent.height, withMsaa ? device->msaaSamples() : VK_SAMPLE_COUNT_1_BIT, imageFormat, tiling, usage, properties, {});
    
    transitionImageLayout(device, commandPool, _image, VK_IMAGE_LAYOUT_UNDEFINED, layout, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels);
    
    if (createImageView) {
        _imageView = std::make_shared<ImageView>(_device, _image, imageFormat, VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

ColorImage::~ColorImage()
{
    
}
