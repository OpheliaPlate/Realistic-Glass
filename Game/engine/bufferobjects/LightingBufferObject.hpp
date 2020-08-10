#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "Buffer.hpp"
#include "Descriptor.hpp"
#include "Device.hpp"

class LightingBufferObject
{
public:
    struct Structure
    {
        alignas(4) glm::vec4 viewPosition;
        alignas(4) glm::vec4 lightPosition;
        alignas(4) glm::vec4 lightColor;
        alignas(4) int glassAlgo;
    };
    
public:
    LightingBufferObject(std::shared_ptr<Device> device, uint32_t binding, VkShaderStageFlagBits stageFlags)
        : _device(device)
        , _descriptor{binding,
                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      stageFlags}
    {
        
    }
    
    ~LightingBufferObject()
    {
        
    }
    
    void resize(size_t size)
    {
        _buffers.clear();
        _buffers.resize(size);
        for (size_t i = 0; i < size; i++) {
            _buffers[i] = std::make_shared<Buffer>(_device, sizeof(Structure), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }
    
    std::shared_ptr<Buffer> buffer(size_t index)
    {
        return _buffers[index];
    }
    
    const Descriptor& descriptor()
    {
        return _descriptor;
    }
    
private:
    const Descriptor _descriptor;

    std::vector<std::shared_ptr<Buffer>> _buffers;
    
    std::shared_ptr<Device> _device;
};
