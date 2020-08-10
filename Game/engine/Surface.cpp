#include "Surface.hpp"

#include "Instance.hpp"
#include "Window.hpp"

class Surface::Private
{
public:
    Private(std::shared_ptr<Window> window, std::shared_ptr<Instance> instance)
        : _instance(instance)
        , _surface(window->createWindowSurface(instance))
    {
        
    }
    
    ~Private()
    {
        vkDestroySurfaceKHR(*_instance, _surface, nullptr);
    }
    
    operator VkSurfaceKHR()
    {
        return _surface;
    }
    
private:
    VkSurfaceKHR _surface;
    
    std::shared_ptr<Instance> _instance;
};

Surface::Surface(std::shared_ptr<Window> window, std::shared_ptr<Instance> instance)
    : _delegate(std::make_unique<Private>(window, instance))
{
    
}

Surface::~Surface()
{
    
}

Surface::operator VkSurfaceKHR()
{
    return *_delegate;
}
