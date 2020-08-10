#pragma once

#include "Pass.hpp"

#include <vulkan/vulkan.h>

#include <array>
#include <memory>
#include <vector>
#include <string>

#include "ColorImage.hpp"
#include "DepthImage.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "Framebuffer.hpp"
#include "LightingBufferObject.hpp"
#include "OffscreenRenderPass.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "SwapChainImage.hpp"
#include "UniformBufferObject.hpp"

class Device;

class StencilPass : public Pass
{
public:
    StencilPass(std::shared_ptr<Device> device,
                std::shared_ptr<CommandPool> commandPool, 
                std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
                std::shared_ptr<DescriptorPool> descriptorPool,
                VkFormat imageFormat,
                VkExtent2D extent,
                const std::string vertShader,
                const std::string fragShader,
                std::vector<std::shared_ptr<SwapChainImage>> swapChainImages,
                std::shared_ptr<UniformBufferObject> uniformBufferObject);
    
    ~StencilPass();
    
    std::shared_ptr<ColorImage> stencilImage();
        
private:
    void createFramebuffers(std::vector<std::shared_ptr<SwapChainImage>> swapChainImages);
    
    void createDescriptorSets(std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
                              std::shared_ptr<DescriptorPool> descriptorPool,
                              std::vector<std::shared_ptr<SwapChainImage>> swapChainImages,
                              std::shared_ptr<UniformBufferObject> uniformBufferObject);
    
private:
    std::shared_ptr<ColorImage> _stencilImage;
};
