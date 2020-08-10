#pragma once

#include <vulkan/vulkan.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "InstanceBufferObject.hpp"

#include <array>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec3 color;
    glm::vec2 texCoord;

    static std::array<VkVertexInputBindingDescription, 2> getBindingDescription() {
        std::array<VkVertexInputBindingDescription, 2> bindingDescriptions{};
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(InstanceBufferObject::Structure);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescriptions;
    }

    static std::array<VkVertexInputAttributeDescription, 13> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 13> attributeDescriptions{};
        
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, norm);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, color);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, texCoord);
        
        attributeDescriptions[4].binding = 1;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[4].offset = offsetof(InstanceBufferObject::Structure, id);
        
        attributeDescriptions[5].binding = 1;
        attributeDescriptions[5].location = 5;
        attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[5].offset = offsetof(InstanceBufferObject::Structure, pos);

        attributeDescriptions[6].binding = 1;
        attributeDescriptions[6].location = 6;
        attributeDescriptions[6].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[6].offset = offsetof(InstanceBufferObject::Structure, rot);

        attributeDescriptions[7].binding = 1;
        attributeDescriptions[7].location = 7;
        attributeDescriptions[7].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[7].offset = offsetof(InstanceBufferObject::Structure, scale);

        attributeDescriptions[8].binding = 1;
        attributeDescriptions[8].location = 8;
        attributeDescriptions[8].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[8].offset = offsetof(InstanceBufferObject::Structure, texIndex);

        attributeDescriptions[9].binding = 1;
        attributeDescriptions[9].location = 9;
        attributeDescriptions[9].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[9].offset = offsetof(InstanceBufferObject::Structure, selected);

        attributeDescriptions[10].binding = 1;
        attributeDescriptions[10].location = 10;
        attributeDescriptions[10].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[10].offset = offsetof(InstanceBufferObject::Structure, fresnel);
        
        attributeDescriptions[11].binding = 1;
        attributeDescriptions[11].location = 11;
        attributeDescriptions[11].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[11].offset = offsetof(InstanceBufferObject::Structure, color);

        attributeDescriptions[12].binding = 1;
        attributeDescriptions[12].location = 12;
        attributeDescriptions[12].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[12].offset = offsetof(InstanceBufferObject::Structure, refraction);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && norm == other.norm && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ ((hash<glm::vec3>()(vertex.color) << 1) ^ (hash<glm::vec3>()(vertex.norm) << 2))) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

