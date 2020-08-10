#include "Object.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "model/tiny_obj_loader.h"

#include <unordered_map>

#include "CommandPool.hpp"
#include "Device.hpp"
#include "EfficientBuffer.hpp"
#include "Vertex.hpp"

class Object::Private
{
public:
    Private(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, const std::string path, uint32_t textureId, uint32_t amountOfInstances)
    : _position(glm::mat4(1.0f))
    , _device(device)
    {
        loadModel(path);
        
        _vertexBuffer = std::make_shared<EfficientBuffer>(device, commandPool, _vertices);
        _indexBuffer = std::make_shared<EfficientBuffer>(device, commandPool, _indices);
        
        _instanceData.resize(amountOfInstances);
        for (int i = 0; i < amountOfInstances; ++i) {
            static uint32_t id = 1;
            _instanceData[i].texIndex = textureId;
            _instanceData[i].id = id;
            id++;
        }
    }
    
    ~Private()
    {
        
    }

    std::shared_ptr<EfficientBuffer> vertexBuffer()
    {
        return _vertexBuffer;
    }

    std::shared_ptr<EfficientBuffer> indexBuffer()
    {
        return _indexBuffer;
    }
    
    std::shared_ptr<TextureImage> textureImage()
    {
        return _textureImage;
    }

    InstanceBufferObject::Structure& instanceData(uint32_t instance)
    {
        return _instanceData[instance];
    }
    
    void setPosition(uint32_t instance, glm::vec3 position)
    {
        _instanceData[instance].pos = position;
    }
    
    void setTextureId(uint32_t instance, uint32_t textureId)
    {
        _instanceData[instance].texIndex = textureId;
    }
    
    uint32_t amountOfInstances()
    {
        return static_cast<uint32_t>(_instanceData.size());
    }
    
    std::shared_ptr<InstanceBufferObject> instanceBufferObject()
    {
        return _instanceBufferObject;
    }
    
    void resetInstanceBufferObject(size_t amountOfFramebuffers)
    {
        _instanceBufferObject = std::make_shared<InstanceBufferObject>(_device, amountOfFramebuffers, _instanceData.size());
    }
    
    void updateInstanceBufferObject(uint32_t currentImage)
    {
        void* data;
        vkMapMemory(*_device, _instanceBufferObject->buffer(currentImage)->memory(), 0, _instanceData.size() * sizeof(InstanceBufferObject::Structure), 0, &data);
            memcpy(data, _instanceData.data(), _instanceData.size() * sizeof(InstanceBufferObject::Structure));
        vkUnmapMemory(*_device, _instanceBufferObject->buffer(currentImage)->memory());
    }
    
private:
    void loadModel(const std::string path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                
                vertex.norm = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
                    _vertices.push_back(vertex);
                }

                _indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
    
private:
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    
    std::shared_ptr<EfficientBuffer> _vertexBuffer;
    std::shared_ptr<EfficientBuffer> _indexBuffer;
    std::shared_ptr<TextureImage> _textureImage;
    
    glm::mat4 _position;
    
    std::vector<InstanceBufferObject::Structure> _instanceData;
    std::shared_ptr<InstanceBufferObject> _instanceBufferObject;
    
    std::shared_ptr<Device> _device;
};

Object::Object(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, const std::string path, uint32_t textureId, uint32_t amountOfInstances)
    : _delegate(std::make_unique<Private>(device, commandPool, path, textureId, amountOfInstances))
{
    
}

Object::~Object()
{
    
}

std::shared_ptr<EfficientBuffer> Object::vertexBuffer()
{
    return _delegate->vertexBuffer();
}

std::shared_ptr<EfficientBuffer> Object::indexBuffer()
{
    return _delegate->indexBuffer();
}

InstanceBufferObject::Structure& Object::instanceData(uint32_t instance)
{
    return _delegate->instanceData(instance);
}

void Object::setPosition(uint32_t instance, glm::vec3 position)
{
    _delegate->setPosition(instance, position);
}

void Object::setTextureId(uint32_t instance, uint32_t textureId)
{
    _delegate->setTextureId(instance, textureId);
}

uint32_t Object::amountOfInstances()
{
    return _delegate->amountOfInstances();
}

std::shared_ptr<InstanceBufferObject> Object::instanceBufferObject()
{
    return _delegate->instanceBufferObject();
}

void Object::resetInstanceBufferObject(size_t amountOfFramebuffers)
{
    _delegate->resetInstanceBufferObject(amountOfFramebuffers);
}

void Object::updateInstanceBufferObject(uint32_t currentImage)
{
    _delegate->updateInstanceBufferObject(currentImage);
}
