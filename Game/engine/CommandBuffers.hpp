#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class CommandPool;
class CubeMapImage;
class Device;
class InstanceBufferObject;
class OffscreenPass;
class Scene;
class ScenePass;
class StencilPass;

class CommandBuffers
{
public:
    CommandBuffers(std::shared_ptr<Device> device,
                   std::shared_ptr<CommandPool> commandPool,
                   std::shared_ptr<ScenePass> scenePass,
                   std::shared_ptr<OffscreenPass> offscreenPass,
                   std::shared_ptr<OffscreenPass> environmentMapPass,
                   std::shared_ptr<StencilPass> stencilPass,
                   std::shared_ptr<Scene> scene,
                   std::shared_ptr<CubeMapImage> cubeMapImage,
                   std::vector<std::shared_ptr<CubeMapImage>> environmentMapImages);
    ~CommandBuffers();
    
    VkCommandBuffer& get(size_t index);
    
public:
    static std::shared_ptr<VkCommandBuffer> beginSingleTimeCommands(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool);
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
