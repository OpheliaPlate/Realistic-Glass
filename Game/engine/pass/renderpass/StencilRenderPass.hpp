#pragma once

#include "RenderPass.hpp"

#include <vulkan/vulkan.h>

#include <memory>

class Device;

class StencilRenderPass : public RenderPass
{
public:
    StencilRenderPass(std::shared_ptr<Device> device, VkFormat imageFormat);
    ~StencilRenderPass();
};
