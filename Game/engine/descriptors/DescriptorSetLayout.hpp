#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

#include "Descriptor.hpp"

class Device;

class DescriptorSetLayout
{
public:
    DescriptorSetLayout(std::shared_ptr<Device> device, const std::vector<Descriptor>& descriptors);
    ~DescriptorSetLayout();
    
    operator VkDescriptorSetLayout();
    operator VkDescriptorSetLayout*();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
