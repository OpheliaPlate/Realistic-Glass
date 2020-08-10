#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include "Descriptor.hpp"

class Device;
class Surface;

class DescriptorPool
{
public:
    DescriptorPool(std::shared_ptr<Device> device, const std::vector<Descriptor>& descriptors, size_t amountOfImages);
    ~DescriptorPool();
    
    operator VkDescriptorPool();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
