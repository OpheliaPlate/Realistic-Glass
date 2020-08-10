#pragma once

#include <vulkan/vulkan.h>

#include <functional>
#include <memory>
#include <vector>

class Instance;
class Surface;

class Window
{
public:
    struct FramebufferSize
    {
        int width;
        int height;
    };
    
public:
    Window();
    ~Window();
    
    VkSurfaceKHR createWindowSurface(std::shared_ptr<Instance>);
    std::vector<const char*> getRequiredExtensions();
    void run(std::function<void()> drawCallback);
    
    const FramebufferSize framebufferSize();
    void waitForRestore();
    bool readAndResetWindowResizedFlag();
    void registerKeyCallback(int key, std::function<void()> callback);
    void registerMouseMoveCallback(std::function<void(float, float)> callback);
    void registerMouseClickedCallback(std::function<void(int, int)> callback);
    void handleInput();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
