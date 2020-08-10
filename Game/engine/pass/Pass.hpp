#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class ColorImage;
class DepthImage;
class Device;
class Framebuffer;
class Pipeline;
class PipelineLayout;
class RenderPass;

class Pass
{
protected:
    Pass(std::shared_ptr<Device> device, VkExtent2D extent);
    ~Pass();
    
public:
    size_t amountOfFramebuffers();
    VkExtent2D extent();
    std::shared_ptr<RenderPass> renderPass();
    std::shared_ptr<PipelineLayout> pipelineLayout();
    std::shared_ptr<Pipeline> pipeline();
    std::shared_ptr<ColorImage> colorImage();
    std::shared_ptr<Framebuffer> framebuffers(size_t index);
    VkDescriptorSet& descriptorSet(size_t index);
    
protected:
    VkExtent2D _extent;
    
    std::shared_ptr<RenderPass> _renderPass;
    std::shared_ptr<PipelineLayout> _pipelineLayout;
    std::shared_ptr<Pipeline> _pipeline;
    std::shared_ptr<ColorImage> _colorImage;
    std::shared_ptr<DepthImage> _depthImage;
    std::vector<std::shared_ptr<Framebuffer>> _framebuffers;
    std::vector<VkDescriptorSet> _descriptorSets;
    
    std::shared_ptr<Device> _device;
};
