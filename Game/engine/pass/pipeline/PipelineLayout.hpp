#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class DescriptorSetLayout;
class Device;

class PipelineLayout
{
public:
    PipelineLayout(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::vector<VkPushConstantRange> pushConstantRanges);
    ~PipelineLayout();
    
public:
    operator VkPipelineLayout();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
