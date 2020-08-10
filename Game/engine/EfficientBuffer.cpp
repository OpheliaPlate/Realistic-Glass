#include "EfficientBuffer.hpp"

#include <exception>

#include "Buffer.hpp"
#include "CommandPool.hpp"
#include "Device.hpp"
#include "Vertex.hpp"

class EfficientBuffer::Private
{
    private:
        Private(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, size_t size, size_t amount, const void* srcData, uint32_t type)
            : _amount(amount)
        {
            VkDeviceSize bufferSize = size * amount;
            auto stagingBuffer = std::make_shared<Buffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            
            void* data;
            vkMapMemory(*device, stagingBuffer->memory(), 0, bufferSize, 0, &data);
                memcpy(data, srcData, (size_t) bufferSize);
            vkUnmapMemory(*device, stagingBuffer->memory());
            
            _vertexBuffer = std::make_shared<Buffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | type, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            
            Buffer::copyBuffer(device, commandPool, stagingBuffer, _vertexBuffer, bufferSize);
        }
    
public:
    Private(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, const std::vector<Vertex> vertices)
        : Private(device, commandPool, sizeof(vertices[0]), vertices.size(), vertices.data(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    {
        
    }
    
    Private(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, const std::vector<uint32_t> indices)
        : Private(device, commandPool, sizeof(indices[0]), indices.size(), indices.data(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    {
        
    }
    
    ~Private()
    {
        
    }
    
    operator VkBuffer()
    {
        return *_vertexBuffer;
    }
    
    size_t amount()
    {
        return _amount;
    }

private:
    std::shared_ptr<Buffer> _vertexBuffer;
    
    size_t _amount;
};

EfficientBuffer::EfficientBuffer(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, const std::vector<Vertex> vertices)
    : _delegate(std::make_unique<Private>(device, commandPool, vertices))
{
    
}

EfficientBuffer::EfficientBuffer(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, const std::vector<uint32_t> indices)
    : _delegate(std::make_unique<Private>(device, commandPool, indices))
{
    
}

EfficientBuffer::~EfficientBuffer()
{
    
}

EfficientBuffer::operator VkBuffer()
{
    return *_delegate;
}

size_t EfficientBuffer::amount()
{
    return _delegate->amount();
}


