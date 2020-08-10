#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class Device;

class ShaderModule
{
public:
    ShaderModule(std::shared_ptr<Device> device, const std::string& path);
    ~ShaderModule();
    
    operator VkShaderModule();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
