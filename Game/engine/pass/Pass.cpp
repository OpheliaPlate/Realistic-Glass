#include "Pass.hpp"

Pass::Pass(std::shared_ptr<Device> device, VkExtent2D extent)
    : _device(device)
    , _extent(extent)
{
    
}

Pass::~Pass()
{
    
}

size_t Pass::amountOfFramebuffers()
{
    return _framebuffers.size();
}
    
VkExtent2D Pass::extent()
{
    return _extent;
}

std::shared_ptr<RenderPass> Pass::renderPass()
{
    return _renderPass;
}

std::shared_ptr<PipelineLayout> Pass::pipelineLayout()
{
    return _pipelineLayout;
}

std::shared_ptr<Pipeline> Pass::pipeline()
{
    return _pipeline;
}

std::shared_ptr<ColorImage> Pass::colorImage()
{
    return _colorImage;
}

std::shared_ptr<Framebuffer> Pass::framebuffers(size_t index)
{
    return _framebuffers[index];
}

VkDescriptorSet& Pass::descriptorSet(size_t index)
{
    return _descriptorSets[index];
}
