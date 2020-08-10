#include "CommandPool.hpp"

#include <exception>

#include "Device.hpp"
#include "Surface.hpp"

class CommandPool::Private
{
public:
    Private(std::shared_ptr<Device> device, std::shared_ptr<Surface> surface)
        : _device(device)
    {
        Device::QueueFamilyIndices queueFamilyIndices = Device::findQueueFamilies(device->physicalDevice(), surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(*_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    
    ~Private()
    {
        vkDestroyCommandPool(*_device, _commandPool, nullptr);
    }
    
    operator VkCommandPool()
    {
        return _commandPool;
    }
    
private:
    VkCommandPool _commandPool;
    
    std::shared_ptr<Device> _device;
};

CommandPool::CommandPool(std::shared_ptr<Device> device, std::shared_ptr<Surface> surface)
    : _delegate(std::make_unique<Private>(device, surface))
{
    
}

CommandPool::~CommandPool()
{
    
}

CommandPool::operator VkCommandPool()
{
    return *_delegate;
}
