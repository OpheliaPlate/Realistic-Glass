#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "InstanceBufferObject.hpp"

class CommandPool;
class Device;
class EfficientBuffer;
class InstanceBufferObject;
class TextureImage;

class Object
{
public:
    Object(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool, const std::string path, uint32_t textureId, uint32_t amountOfInstances);
    ~Object();
    
    std::shared_ptr<EfficientBuffer> vertexBuffer();
    std::shared_ptr<EfficientBuffer> indexBuffer();
    InstanceBufferObject::Structure& instanceData(uint32_t instance);
    void setPosition(uint32_t instance, glm::vec3 position);
    void setTextureId(uint32_t instance, uint32_t textureId);
    uint32_t amountOfInstances();
    std::shared_ptr<InstanceBufferObject> instanceBufferObject();
    void resetInstanceBufferObject(size_t amountOfFramebuffers);
    void updateInstanceBufferObject(uint32_t currentImage);
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
