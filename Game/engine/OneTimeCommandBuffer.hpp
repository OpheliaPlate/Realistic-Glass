#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class CommandPool;
class Device;

class OneTimeCommandBuffer
{
public:
    OneTimeCommandBuffer(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool);
    ~OneTimeCommandBuffer();
    
    operator VkCommandBuffer&();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
