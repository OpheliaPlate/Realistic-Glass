#include "SwapChain.hpp"

#include <array>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>

#include "Buffer.hpp"
#include "Camera.hpp"
#include "CommandPool.hpp"
#include "CubeMapImage.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "EfficientBuffer.hpp"
#include "ImageView.hpp"
#include "InstanceBufferObject.hpp"
#include "OffscreenPass.hpp"
#include "OneTimeCommandBuffer.hpp"
#include "Scene.hpp"
#include "ScenePass.hpp"
#include "StencilPass.hpp"
#include "Surface.hpp"
#include "SwapChainImage.hpp"
#include "TextureImage.hpp"
#include "Window.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class SwapChain::Private
{
public:
    Private(std::shared_ptr<Device> device, std::shared_ptr<Surface> surface, std::shared_ptr<CommandPool> commandPool, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::shared_ptr<Scene> scene, const Window::FramebufferSize& framebufferSize)
        : _device(device)
        , _scene(scene)
    {
        VkFormat imageFormat;
        createSwapChain(_device, surface, framebufferSize, imageFormat);
        
        scene->resetInstanceBufferObjects(_swapChainImages.size());
        scene->uniformBufferObject()->resize(_swapChainImages.size());
        scene->lightingBufferObject()->resize(_swapChainImages.size());
        
        _descriptorPool = std::make_shared<DescriptorPool>(_device, scene->descriptors(), _swapChainImages.size());
        
        _scenePass = std::make_shared<ScenePass>(device, commandPool, descriptorSetLayout, _descriptorPool, imageFormat, _extent, "vert.spv", "frag.spv", _swapChainImages, _scene->uniformBufferObject(), _scene->lightingBufferObject(), _scene->cubeMapImage(), _scene->environmentMapImages(), scene->textureImages());
        
        _offscreenPass = std::make_shared<OffscreenPass>(device, commandPool, descriptorSetLayout, _descriptorPool, VK_FORMAT_R32_SFLOAT, VkExtent2D{1024, 1024}, "offscreenvert.spv", "offscreenfrag.spv", _swapChainImages, _scene->uniformBufferObject(), _scene->lightingBufferObject(), _scene->cubeMapImage(), _scene->environmentMapImages(), scene->textureImages());
        
        _environmentMapPass = std::make_shared<OffscreenPass>(device, commandPool, descriptorSetLayout, _descriptorPool, VK_FORMAT_R32G32B32A32_SFLOAT, VkExtent2D{1024, 1024}, "environmentmapvert.spv", "environmentmapfrag.spv", _swapChainImages, _scene->uniformBufferObject(), _scene->lightingBufferObject(), _scene->cubeMapImage(), _scene->environmentMapImages(), scene->textureImages());
        
        _stencilPass = std::make_shared<StencilPass>(device, commandPool, descriptorSetLayout, _descriptorPool, VK_FORMAT_R32_SFLOAT, _extent, "stencilvert.spv", "stencilfrag.spv", _swapChainImages, _scene->uniformBufferObject());
    
        _commandBuffers = std::make_shared<CommandBuffers>(_device, commandPool, _scenePass, _offscreenPass, _environmentMapPass, _stencilPass, scene, _scene->cubeMapImage(), _scene->environmentMapImages());
    }
    
    ~Private()
    {
        vkDestroySwapchainKHR(*_device, _swapChain, nullptr);
    }
    
    operator VkSwapchainKHR()
    {
        return _swapChain;
    }
    
    std::vector<std::shared_ptr<SwapChainImage>> images()
    {
        return _swapChainImages;
    }
    
    std::shared_ptr<CommandBuffers> commandBuffers()
    {
        return _commandBuffers;
    }
    
    void updateUniformBuffer(std::shared_ptr<Camera> camera, uint32_t currentImage)
    {
        UniformBufferObject::Structure ubo{};
        ubo.model = glm::mat4(1.0f);
        ubo.view = camera->view();
        ubo.proj = glm::perspective(glm::radians(45.0f), _extent.width / (float) _extent.height, 0.1f, 1024.0f);
        ubo.proj[1][1] *= -1;
        ubo.eye = glm::vec4(camera->position(), 1.0f);
        
        auto spherePosition1 = _scene->sphere()->instanceData(0).pos;
        auto spherePosition2 = _scene->sphere()->instanceData(1).pos;
        auto spherePosition3 = _scene->sphere()->instanceData(2).pos;
        auto spherePosition4 = _scene->sphere()->instanceData(3).pos;
        ubo.referencePoints[0] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -0.1, -1.8));
        ubo.referencePoints[1] = glm::translate(glm::mat4(1.0f), glm::vec3(-spherePosition1.x, -spherePosition1.y, -spherePosition1.z));
        ubo.referencePoints[2] = glm::translate(glm::mat4(1.0f), glm::vec3(-spherePosition2.x, -spherePosition2.y, -spherePosition2.z));
        ubo.referencePoints[3] = glm::translate(glm::mat4(1.0f), glm::vec3(-spherePosition3.x, -spherePosition3.y, -spherePosition3.z));
        ubo.referencePoints[4] = glm::translate(glm::mat4(1.0f), glm::vec3(-spherePosition4.x, -spherePosition4.y, -spherePosition4.z));
        
        void* data;
        vkMapMemory(*_device, _scene->uniformBufferObject()->buffer(currentImage)->memory(), 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(*_device, _scene->uniformBufferObject()->buffer(currentImage)->memory());
    }
    
    void updateLightingBuffer(std::shared_ptr<Camera> camera, glm::vec3 position, uint32_t currentImage, uint32_t glassAlgo)
    {
        LightingBufferObject::Structure lbo{};
        lbo.viewPosition = glm::vec4(camera->position(), 1.0f);
        lbo.lightPosition = glm::vec4(position, 1.0f);
        lbo.lightColor = glm::vec4(1.0f, 0.9f, 0.6f, 1.0f);
        lbo.glassAlgo = glassAlgo;

        void* data;
        vkMapMemory(*_device, _scene->lightingBufferObject()->buffer(currentImage)->memory(), 0, sizeof(lbo), 0, &data);
            memcpy(data, &lbo, sizeof(lbo));
        vkUnmapMemory(*_device, _scene->lightingBufferObject()->buffer(currentImage)->memory());
    }
    
    int32_t getSelectedId(std::shared_ptr<CommandPool> commandPool, int x, int y)
    {
        // Make sure color writes to the framebuffer are finished before using it as transfer source
        Image::transitionImageLayout(
            _device,
            commandPool,
            *_stencilPass->colorImage(),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT,
            1);

        VkImageSubresourceRange cubeFaceSubresourceRange = {};
        cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cubeFaceSubresourceRange.baseMipLevel = 0;
        cubeFaceSubresourceRange.levelCount = 1;
        cubeFaceSubresourceRange.baseArrayLayer = 0;
        cubeFaceSubresourceRange.layerCount = 1;

        // Change image layout of one cubemap face to transfer destination
        Image::transitionImageLayout(
            _device,
            commandPool,
            *_stencilPass->stencilImage(),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT,
            1);

        // Define the region to blit (we will blit the whole swapchain image)
        VkOffset3D blitSrc;
        blitSrc.x = x*2;
        blitSrc.y = y*2;
        blitSrc.z = 0;
        
        VkOffset3D blitDst;
        blitDst.x = 0;
        blitDst.y = 0;
        blitDst.z = 0;
        
        VkOffset3D blitSize;
        blitSize.x = 1;
        blitSize.y = 1;
        blitSize.z = 1;
        
        {
            auto commandBuffer = std::make_shared<OneTimeCommandBuffer>(_device, commandPool);
            
            // Otherwise use image copy (requires us to manually flip components)
            VkImageCopy imageCopyRegion{};
            imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.srcSubresource.layerCount = 1;
            imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.dstSubresource.layerCount = 1;
            imageCopyRegion.srcOffset = blitSrc;
            imageCopyRegion.dstOffset = blitDst;
            imageCopyRegion.extent.width = 1;
            imageCopyRegion.extent.height = 1;
            imageCopyRegion.extent.depth = 1;

            // Issue the copy command
            vkCmdCopyImage(
                *commandBuffer,
                *_stencilPass->colorImage(),
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                *_stencilPass->stencilImage(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &imageCopyRegion);
        }

        // Transform framebuffer color attachment back
        Image::transitionImageLayout(
            _device,
            commandPool,
            *_stencilPass->colorImage(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT,
            1);

        // Change image layout of copied face to shader read
        Image::transitionImageLayout(
            _device,
            commandPool,
            *_stencilPass->stencilImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_ASPECT_COLOR_BIT,
            1);
        
        //std::ofstream file("/Users/andrei/Documents/test.ppm", std::ios::out | std::ios::binary);
        //file << "P6\n" << _extent.width << "\n" << _extent.height << "\n" << 255 << "\n";
        
        const char* data;
        vkMapMemory(*_device, _stencilPass->stencilImage()->memory(), 0, VK_WHOLE_SIZE, 0, (void**)&data);
        int selectedId = static_cast<int32_t>(reinterpret_cast<const float*>(data)[0]);
        vkUnmapMemory(*_device, _stencilPass->stencilImage()->memory());
        
        return selectedId;
    }
    
private:
    void createSwapChain(std::shared_ptr<Device> device, std::shared_ptr<Surface> surface, const Window::FramebufferSize& framebufferSize, VkFormat& imageFormat)
    {
        Device::SwapChainSupportDetails swapChainSupport = Device::querySwapChainSupport(device->physicalDevice(), surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(framebufferSize, swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = *surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        Device::QueueFamilyIndices indices = Device::findQueueFamilies(device->physicalDevice(), surface);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(*device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        std::vector<VkImage> swapChainImages;
        vkGetSwapchainImagesKHR(*device, _swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(*device, _swapChain, &imageCount, swapChainImages.data());
        for (int i = 0; i < imageCount; ++i) {
            _swapChainImages.emplace_back(std::make_shared<SwapChainImage>(device, swapChainImages[i], surfaceFormat.format));
        }
        
        imageFormat = surfaceFormat.format;
        _extent = extent;
    }
    
private:
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const Window::FramebufferSize& framebufferSize, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(framebufferSize.width),
                static_cast<uint32_t>(framebufferSize.height)
            };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }
    
private:
    VkSwapchainKHR _swapChain;
    VkExtent2D _extent;
    
    std::vector<std::shared_ptr<SwapChainImage>> _swapChainImages;
    std::shared_ptr<DescriptorPool> _descriptorPool;
    
    std::shared_ptr<ScenePass> _scenePass;
    std::shared_ptr<OffscreenPass> _offscreenPass;
    std::shared_ptr<OffscreenPass> _environmentMapPass;
    std::shared_ptr<StencilPass> _stencilPass;
    
    std::shared_ptr<CommandBuffers> _commandBuffers;
    
    std::shared_ptr<Scene> _scene;
    std::shared_ptr<Device> _device;
};


SwapChain::SwapChain(std::shared_ptr<Device> device, std::shared_ptr<Surface> surface, std::shared_ptr<CommandPool> commandPool, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::shared_ptr<Scene> scene, const Window::FramebufferSize& framebufferSize)
    : _delegate(std::make_unique<Private>(device, surface, commandPool, descriptorSetLayout, scene, framebufferSize))
{
    
}

SwapChain::~SwapChain()
{
    
}

SwapChain::operator VkSwapchainKHR()
{
    return *_delegate;
}

std::vector<std::shared_ptr<SwapChainImage>> SwapChain::images()
{
    return _delegate->images();
}

std::shared_ptr<CommandBuffers> SwapChain::commandBuffers()
{
    return _delegate->commandBuffers();
}

void SwapChain::updateUniformBuffer(std::shared_ptr<Camera> camera, uint32_t currentImage)
{
    _delegate->updateUniformBuffer(camera, currentImage);
}

void SwapChain::updateLightingBuffer(std::shared_ptr<Camera> camera, glm::vec3 position, uint32_t currentImage, uint32_t glassAlgo)
{
    _delegate->updateLightingBuffer(camera, position, currentImage, glassAlgo);
}

int32_t SwapChain::getSelectedId(std::shared_ptr<CommandPool> commandPool, int x, int y)
{
    return _delegate->getSelectedId(commandPool, x, y);
}
