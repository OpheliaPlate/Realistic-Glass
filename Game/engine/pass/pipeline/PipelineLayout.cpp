#include "PipelineLayout.hpp"

#include "DescriptorSetLayout.hpp"
#include "Device.hpp"

class PipelineLayout::Private
{
public:
    Private(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::vector<VkPushConstantRange> pushConstantRanges)
        : _device(device)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = *descriptorSetLayout;

        if (!pushConstantRanges.empty()) {
            pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
            pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
        }
        
        if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }
    
    ~Private()
    {
        vkDestroyPipelineLayout(*_device, _pipelineLayout, nullptr);
    }

    operator VkPipelineLayout()
    {
        return _pipelineLayout;
    }
    
private:
    VkPipelineLayout _pipelineLayout;
    
    std::shared_ptr<Device> _device;
};

PipelineLayout::PipelineLayout(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::vector<VkPushConstantRange> pushConstantRanges)
    : _delegate(std::make_unique<Private>(device, descriptorSetLayout, pushConstantRanges))
{
    
}

PipelineLayout::~PipelineLayout()
{
    
}

PipelineLayout::operator VkPipelineLayout()
{
    return *_delegate;
}
