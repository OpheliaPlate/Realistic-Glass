#include "DepthImage.hpp"

#include <exception>

#include "CommandPool.hpp"
#include "Device.hpp"
#include "ImageView.hpp"

DepthImage::DepthImage(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, VkExtent2D extent, bool withMsaa)
    : Image(device, 0, VK_SHADER_STAGE_FRAGMENT_BIT)
{
    VkFormat depthFormat = findDepthFormat(device);

    createImage(extent.width, extent.height, withMsaa ? device->msaaSamples() : VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, {});
    
    transitionImageLayout(device, commandPool, _image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT, _mipLevels);
    
    _imageView = std::make_shared<ImageView>(_device, _image, depthFormat, VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

DepthImage::~DepthImage()
{
    
}

VkFormat DepthImage::findDepthFormat(std::shared_ptr<Device> device) {
    return findSupportedFormat(device,
    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat DepthImage::findSupportedFormat(std::shared_ptr<Device> device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device->physicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}
