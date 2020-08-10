#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>

class Device;
class PipelineLayout;
class RenderPass;

class Pipeline
{
public:
    Pipeline(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass, std::shared_ptr<PipelineLayout> pipelineLayout, VkExtent2D extent, const std::string vertShader, const std::string fragShader, bool useEmptyVertexInputInfo, bool withMsaa, VkCullModeFlagBits cullMode);
    ~Pipeline();
    
public:
    operator VkPipeline();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
