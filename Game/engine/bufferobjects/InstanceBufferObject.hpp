#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "Buffer.hpp"
#include "Device.hpp"

class InstanceBufferObject
{
public:
    struct Structure
    {
        alignas(4) glm::uint32_t id;
        alignas(4) glm::vec3 pos;
        alignas(4) glm::vec3 rot;
        alignas(4) glm::float32_t scale;
        alignas(4) glm::uint32_t texIndex;
        alignas(4) glm::uint32_t selected;
        alignas(4) glm::float32_t fresnel;
        alignas(4) glm::vec3 color;
        alignas(4) glm::float32_t refraction;
    };
    
public:
    InstanceBufferObject(std::shared_ptr<Device> device, size_t size, uint32_t amountOfInstances)
    {
        _buffers.resize(size);
        for (size_t i = 0; i < size; i++) {
            _buffers[i] = std::make_shared<Buffer>(device, sizeof(Structure) * amountOfInstances, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }
    
    ~InstanceBufferObject()
    {
        
    }
    
    std::shared_ptr<Buffer> buffer(size_t index)
    {
        return _buffers[index];
    }
    
private:
    std::vector<std::shared_ptr<Buffer>> _buffers;
};
