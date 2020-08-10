#include "DescriptorSetLayout.hpp"

#include <array>
#include <exception>

#include "Device.hpp"

class DescriptorSetLayout::Private
{
public:
    Private(std::shared_ptr<Device> device, const std::vector<Descriptor>& descriptors)
        : _device(device)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        
        for (const auto& descriptor : descriptors) {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = descriptor.binding;
            binding.descriptorCount = 1;
            binding.descriptorType = descriptor.descriptorType;
            binding.pImmutableSamplers = nullptr;
            binding.stageFlags = descriptor.stageFlags;
            bindings.emplace_back(binding);
        }
        
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(*_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
    
    ~Private()
    {
        vkDestroyDescriptorSetLayout(*_device, _descriptorSetLayout, nullptr);
    }
    
    operator VkDescriptorSetLayout()
    {
        return _descriptorSetLayout;
    }
    
    operator VkDescriptorSetLayout*()
    {
        return &_descriptorSetLayout;
    }
    
private:
    VkDescriptorSetLayout _descriptorSetLayout;
    
    std::shared_ptr<Device> _device;
};

DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device> device, const std::vector<Descriptor>& descriptors)
    : _delegate(std::make_unique<Private>(device, descriptors))
{
    
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    
}

DescriptorSetLayout::operator VkDescriptorSetLayout()
{
    return *_delegate;
}

DescriptorSetLayout::operator VkDescriptorSetLayout*()
{
    return *_delegate;
}
