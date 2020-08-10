#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class Device;

class RenderPass
{
protected:
    RenderPass(std::shared_ptr<Device> device);
    ~RenderPass();
    
public:
    operator VkRenderPass();
    
protected:
    VkRenderPass _renderPass;

private:
    std::shared_ptr<Device> _device;
};
