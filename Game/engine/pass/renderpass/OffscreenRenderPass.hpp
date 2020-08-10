#pragma once

#include "RenderPass.hpp"

#include <vulkan/vulkan.h>

#include <memory>

class Device;

class OffscreenRenderPass : public RenderPass
{
public:
    OffscreenRenderPass(std::shared_ptr<Device> device, VkFormat imageFormat);
    ~OffscreenRenderPass();
};
