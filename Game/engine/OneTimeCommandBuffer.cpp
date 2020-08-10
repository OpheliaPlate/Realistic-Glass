#include "OneTimeCommandBuffer.hpp"

#include <exception>

#include "CommandPool.hpp"
#include "Device.hpp"

class OneTimeCommandBuffer::Private
{
public:
    Private(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool)
        : _device(device)
        , _commandPool(commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = *commandPool;
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(*device, &allocInfo, &_commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(_commandBuffer, &beginInfo);
    }
    
    ~Private()
    {
        vkEndCommandBuffer(_commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_commandBuffer;

        vkQueueSubmit(_device->graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(_device->graphicsQueue());

        vkFreeCommandBuffers(*_device, *_commandPool, 1, &_commandBuffer);
    }
    
    operator VkCommandBuffer&()
    {
        return _commandBuffer;
    }
    
private:
    VkCommandBuffer _commandBuffer;
    
    std::shared_ptr<Device> _device;
    std::shared_ptr<CommandPool> _commandPool;
};

OneTimeCommandBuffer::OneTimeCommandBuffer(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool)
    : _delegate(std::make_unique<Private>(device, commandPool))
{
    
}

OneTimeCommandBuffer::~OneTimeCommandBuffer()
{
    
}

OneTimeCommandBuffer::operator VkCommandBuffer&()
{
    return *_delegate;
}
