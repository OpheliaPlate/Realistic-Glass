#include "Window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <map>
#include <optional>
#include <set>

#include "Instance.hpp"
#include "Surface.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Window::Private
{
    const std::set<int> kContinuousKeys = {
        GLFW_KEY_W,
        GLFW_KEY_S,
        GLFW_KEY_A,
        GLFW_KEY_D,
        GLFW_KEY_UP,
        GLFW_KEY_DOWN,
        GLFW_KEY_LEFT,
        GLFW_KEY_RIGHT
    };
    
public:
    Private()
    {
        using namespace std::placeholders;
        
        glfwInit();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        
        _window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        
        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, onWindowResized);
        
        //glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetKeyCallback(_window, onKeyPressed);
        glfwSetCursorPosCallback(_window, onInitialMouseMoved);
        glfwSetMouseButtonCallback(_window, onMouseClicked);
    }

    ~Private()
    {
        glfwDestroyWindow(_window);
        
        glfwTerminate();
    }

    VkSurfaceKHR createWindowSurface(std::shared_ptr<Instance> instance)
    {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(*instance, _window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        return surface;
    }
    
    std::vector<const char*> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        
        return extensions;
    }
    
    void run(std::function<void()> drawCallback)
    {
        while (!glfwWindowShouldClose(_window)) {
            glfwPollEvents();
            drawCallback();
        }
    }

    const FramebufferSize framebufferSize()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(_window, &width, &height);
        return {width, height};
    }
    
    void waitForRestore()
    {
        auto size = framebufferSize();
        while (size.width == 0 || size.width == 0) {
            size = framebufferSize();
            glfwWaitEvents();
        }
    }
    
    bool readAndResetWindowResizedFlag()
    {
        if (_windowResized) {
            _windowResized = false;
            return true;
        }
        
        return false;
    }
    
    void registerKeyCallback(int key, std::function<void()> callback)
    {
        _keyboardCallbacks.emplace(key, callback);
    }
    
    void registerMouseMoveCallback(std::function<void(float, float)> callback)
    {
        _mouseMoveCallback = callback;
    }
    
    void registerMouseClickedCallback(std::function<void(int, int)> callback)
    {
        _mouseClickedCallback = callback;
    }
    
    void handleInput()
    {
        std::for_each(_depressedKeys.cbegin(), _depressedKeys.cend(), [this](const auto& key) {
            if (auto entry = _keyboardCallbacks.find(key); entry != _keyboardCallbacks.end()) {
                entry->second();
            }
        });
        
        _depressedKeys.erase(std::remove_if(_depressedKeys.begin(), _depressedKeys.end(), [this](auto& key) {
            return kContinuousKeys.find(key) == kContinuousKeys.end();
        }), _depressedKeys.end());

        float xDelta = _xNext - _xCurrent;
        float yDelta = _yNext - _yCurrent;
        
        if (_mouseMoveCallback) {
            (*_mouseMoveCallback)(xDelta, yDelta);
        }
        
        _xCurrent = _xNext;
        _yCurrent = _yNext;
        
        if (_mouseClickedCallback && _mouseClicked) {
            (*_mouseClickedCallback)(_xCurrent, _yCurrent);
            _mouseClicked = false;
        }
    }
    
private:
    static void onWindowResized(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Private*>(glfwGetWindowUserPointer(window));
        app->_windowResized = true;
    }
    
    static void onKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto app = reinterpret_cast<Private*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS) {
            app->_depressedKeys.emplace_back(key);
        } else if (action == GLFW_RELEASE) {
            app->_depressedKeys.erase(std::remove_if(app->_depressedKeys.begin(), app->_depressedKeys.end(), [key](auto& depressedKey) {
                return depressedKey == key;
            }), app->_depressedKeys.end());
        }
    }
    
    static void onInitialMouseMoved(GLFWwindow* window, double xpos, double ypos)
    {
        auto app = reinterpret_cast<Private*>(glfwGetWindowUserPointer(window));
        app->_xCurrent = xpos;
        app->_yCurrent = ypos;
        app->_xNext = xpos;
        app->_yNext = ypos;
        glfwSetCursorPosCallback(app->_window, onMouseMoved);
    }
    
    static void onMouseMoved(GLFWwindow* window, double xpos, double ypos)
    {
        auto app = reinterpret_cast<Private*>(glfwGetWindowUserPointer(window));
        app->_xNext = xpos;
        app->_yNext = ypos;
    }
    
    static void onMouseClicked(GLFWwindow* window, int button, int action, int mods)
    {
        auto app = reinterpret_cast<Private*>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
            app->_mouseClicked = true;
        }
    }
    
private:
    GLFWwindow* _window;
    
    bool _windowResized{false};
    
    std::vector<int> _depressedKeys;
    std::map<int, std::function<void()>> _keyboardCallbacks;
    
    float _xCurrent{0.0f};
    float _yCurrent{0.0f};
    float _xNext{-0.0f};
    float _yNext{-0.0f};
    std::optional<std::function<void(float, float)>> _mouseMoveCallback;
    
    bool _mouseClicked{false};
    std::optional<std::function<void(int, int)>> _mouseClickedCallback;
};

Window::Window() : _delegate(std::make_unique<Private>())
{
    
}

Window::~Window()
{
    
}

VkSurfaceKHR Window::createWindowSurface(std::shared_ptr<Instance> instance)
{
    return _delegate->createWindowSurface(instance);
}

std::vector<const char*> Window::getRequiredExtensions()
{
    return _delegate->getRequiredExtensions();
}

void Window::run(std::function<void()> drawCallback)
{
    _delegate->run(std::move(drawCallback));
}

const Window::FramebufferSize Window::framebufferSize()
{
    return _delegate->framebufferSize();
}

void Window::waitForRestore()
{
    _delegate->waitForRestore();
}

bool Window::readAndResetWindowResizedFlag()
{
    return _delegate->readAndResetWindowResizedFlag();
}

void Window::registerKeyCallback(int key, std::function<void()> callback)
{
    _delegate->registerKeyCallback(key, callback);
}

void Window::registerMouseMoveCallback(std::function<void(float, float)> callback)
{
    _delegate->registerMouseMoveCallback(callback);
}

void Window::registerMouseClickedCallback(std::function<void(int, int)> callback)
{
    _delegate->registerMouseClickedCallback(callback);
}

void Window::handleInput()
{
    _delegate->handleInput();
}
