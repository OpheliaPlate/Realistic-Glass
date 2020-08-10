#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class CommandPool;
class Device;
class Vertex;

class EfficientBuffer
{
public:
    EfficientBuffer(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, const std::vector<Vertex> vertices);
    EfficientBuffer(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, const std::vector<uint32_t>);
    ~EfficientBuffer();
    
    operator VkBuffer();
    size_t amount();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
