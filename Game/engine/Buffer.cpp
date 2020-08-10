#include "Buffer.hpp"

#include <exception>

#include "CommandPool.hpp"
#include "Device.hpp"
#include "OneTimeCommandBuffer.hpp"

class Buffer::Private
{
public:
    Private(std::shared_ptr<Device> device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
        : _device(device)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(*_device, &bufferInfo, nullptr, &_buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(*_device, _buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = Buffer::findMemoryType(_device, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(*_device, &allocInfo, nullptr, &_bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(*_device, _buffer, _bufferMemory, 0);
    }
    
    ~Private()
    {
        vkDestroyBuffer(*_device, _buffer, nullptr);
        vkFreeMemory(*_device, _bufferMemory, nullptr);
    }
    
    operator VkBuffer()
    {
        return _buffer;
    }
    
    VkDeviceMemory memory()
    {
        return _bufferMemory;
    }

private:
    VkBuffer _buffer;
    VkDeviceMemory _bufferMemory;
    
    std::shared_ptr<Device> _device;
};

Buffer::Buffer(std::shared_ptr<Device> device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : _delegate(std::make_unique<Private>(device, size, usage, properties))
{
    
}

Buffer::~Buffer()
{
    
}

Buffer::operator VkBuffer()
{
    return *_delegate;
}

VkDeviceMemory Buffer::memory()
{
    return _delegate->memory();
}

void Buffer::copyBuffer(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, std::shared_ptr<Buffer> srcBuffer, std::shared_ptr<Buffer> dstBuffer, VkDeviceSize size)
{
    auto commandBuffer = std::make_shared<OneTimeCommandBuffer>(device, commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(*commandBuffer, *srcBuffer, *dstBuffer, 1, &copyRegion);
}

uint32_t Buffer::findMemoryType(std::shared_ptr<Device> device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device->physicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}


