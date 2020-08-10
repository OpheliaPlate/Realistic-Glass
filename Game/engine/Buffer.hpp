#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class CommandPool;
class Device;

class Buffer
{
public:
    Buffer(std::shared_ptr<Device> device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~Buffer();
    
    operator VkBuffer();
    VkDeviceMemory memory();
    
    size_t size();
    
public:
    static void copyBuffer(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, std::shared_ptr<Buffer> srcBuffer, std::shared_ptr<Buffer> dstBuffer, VkDeviceSize size);
    static uint32_t findMemoryType(std::shared_ptr<Device> device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
