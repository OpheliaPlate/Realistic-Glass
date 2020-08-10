#pragma once

#include "Pass.hpp"

#include <vulkan/vulkan.h>

#include <array>
#include <memory>
#include <vector>
#include <string>

class CommandPool;
class CubeMapImage;
class DescriptorPool;
class DescriptorSetLayout;
class Device;
class GraphicsPipeline;
class InstanceBufferObject;
class LightingBufferObject;
class SwapChainImage;
class TextureImage;
class UniformBufferObject;

class ScenePass : public Pass
{
public:
    ScenePass(std::shared_ptr<Device> device,
              std::shared_ptr<CommandPool> commandPool, 
              std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
              std::shared_ptr<DescriptorPool> descriptorPool,
              VkFormat imageFormat,
              VkExtent2D extent,
              const std::string vertShader,
              const std::string fragShader,
              std::vector<std::shared_ptr<SwapChainImage>> swapChainImages,
              std::shared_ptr<UniformBufferObject> uniformBufferObject,
              std::shared_ptr<LightingBufferObject> lightingBufferObject,
              std::shared_ptr<CubeMapImage> cubeMapImage,
              std::vector<std::shared_ptr<CubeMapImage>> environmentMapImages,
              std::vector<std::shared_ptr<TextureImage>> textureImages);
    
    ~ScenePass();
    
private:
    void createFramebuffers(std::vector<std::shared_ptr<SwapChainImage>> swapChainImages);
    
    void createDescriptorSets(std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
                              std::shared_ptr<DescriptorPool> descriptorPool,
                              std::vector<std::shared_ptr<SwapChainImage>> swapChainImages,
                              std::shared_ptr<UniformBufferObject> uniformBufferObject,
                              std::shared_ptr<LightingBufferObject> lightingBufferObject,
                              std::shared_ptr<CubeMapImage> cubeMapImage,
                              std::vector<std::shared_ptr<CubeMapImage>> environmentMapImages,
                              std::vector<std::shared_ptr<TextureImage>> textureImages);
};
