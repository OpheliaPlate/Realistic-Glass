#include "Framebuffer.hpp"

#include <array>

#include "Device.hpp"
#include "RenderPass.hpp"

Framebuffer::Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass, std::vector<VkImageView> attachments, VkExtent2D extent)
    : _device(device)
{
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = *renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(*device, &framebufferInfo, nullptr, &_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(*_device, _framebuffer, nullptr);
}

Framebuffer::operator VkFramebuffer()
{
    return _framebuffer;
}
