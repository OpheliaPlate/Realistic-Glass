#include "RenderPass.hpp"

#include "Device.hpp"

RenderPass::RenderPass(std::shared_ptr<Device> device)
    : _device(device)
{
    
}

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(*_device, _renderPass, nullptr);
}

RenderPass::operator VkRenderPass()
{
    return _renderPass;
}
