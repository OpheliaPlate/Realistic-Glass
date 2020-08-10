#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "CommandBuffers.hpp"
#include "Window.hpp"

class Camera;
class CommandPool;
class DepthImage;
class DescriptorSetLayout;
class Device;
class EfficientBuffer;
class Object;
class Sampler;
class Scene;
class Surface;
class SwapChainImage;
class TextureImage;

class SwapChain
{
public:
    SwapChain(std::shared_ptr<Device> device, std::shared_ptr<Surface> surface, std::shared_ptr<CommandPool> commandPool, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::shared_ptr<Scene> scene, const Window::FramebufferSize& framebufferSize);
    ~SwapChain();
    
    operator VkSwapchainKHR();
    
    std::vector<std::shared_ptr<SwapChainImage>> images();
    std::shared_ptr<CommandBuffers> commandBuffers();
    
    void updateUniformBuffer(std::shared_ptr<Camera> camera, uint32_t currentImage);
    void updateLightingBuffer(std::shared_ptr<Camera> camera, glm::vec3 position, uint32_t currentImage, uint32_t glassAlgo);
    
    int32_t getSelectedId(std::shared_ptr<CommandPool> commandPool, int x, int y);
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
