#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class Instance
{
public:
    Instance(std::vector<const char*> requiredExtensions);
    ~Instance();
    
    operator VkInstance();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
