#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class Device;
class RenderPass;

class Framebuffer
{
public:
    Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass, std::vector<VkImageView> attachments, VkExtent2D extent);
    ~Framebuffer();
    
    operator VkFramebuffer();
    
protected:
    VkFramebuffer _framebuffer;
    
private:
    std::shared_ptr<Device> _device;
};
