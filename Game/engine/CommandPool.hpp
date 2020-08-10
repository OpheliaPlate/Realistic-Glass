#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class Device;
class Surface;

class CommandPool
{
public:
    CommandPool(std::shared_ptr<Device> device, std::shared_ptr<Surface> surface);
    ~CommandPool();
    
    operator VkCommandPool();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
