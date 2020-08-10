#pragma once

#include <vulkan/vulkan.h>

#include <exception>
#include <memory>

class Instance;
class Window;

class Surface
{
public:
    Surface(std::shared_ptr<Window> window, std::shared_ptr<Instance> instance);
    ~Surface();
    
    operator VkSurfaceKHR();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
