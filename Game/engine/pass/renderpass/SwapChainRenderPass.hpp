#pragma once

#include "RenderPass.hpp"

#include <vulkan/vulkan.h>

#include <memory>

class Device;

class SwapChainRenderPass : public RenderPass
{
public:
    SwapChainRenderPass(std::shared_ptr<Device> device, VkFormat imageFormat);
    ~SwapChainRenderPass();
};
