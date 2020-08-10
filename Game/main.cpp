#include <chrono>
#include <iostream>

#include "Camera.hpp"
#include "CommandPool.hpp"
#include "DepthImage.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "EfficientBuffer.hpp"
#include "FrameManager.hpp"
#include "Instance.hpp"
#include "Scene.hpp"
#include "Surface.hpp"
#include "SwapChain.hpp"
#include "SyncObjects.hpp"
#include "TextureImage.hpp"
#include "Window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class HelloTriangleApplication
{
public:
    HelloTriangleApplication()
        : _window(std::make_shared<Window>())
        , _instance(std::make_shared<Instance>(_window->getRequiredExtensions()))
        , _surface(std::make_shared<Surface>(_window, _instance))
        , _device(std::make_shared<Device>(_instance, _surface))
        , _commandPool(std::make_shared<CommandPool>(_device, _surface))
        , _scene(std::make_shared<Scene>(_device, _commandPool))
        , _descriptorSetLayout(std::make_shared<DescriptorSetLayout>(_device, _scene->descriptors()))
        , _swapChain(std::make_shared<SwapChain>(_device, _surface, _commandPool, _descriptorSetLayout, _scene, _window->framebufferSize()))
        , _syncObjects(std::make_shared<SyncObjects>(_device, _swapChain->images().size()))
        , _frameManager(std::make_shared<FrameManager>())
        , _camera(std::make_shared<Camera>(_window))
    {
        _window->registerKeyCallback(GLFW_KEY_SPACE, [this]() {
            _scene->draughts()->move();
        });
        
        _window->registerKeyCallback(GLFW_KEY_L, [this]() {
            _glassAlgo = (_glassAlgo + 1) % 2;
        });
        
        _window->registerKeyCallback(GLFW_KEY_M, [this]() {
            _moving = !_moving;
        });
        
        _window->registerMouseClickedCallback([this](int x, int y) {
            auto selectedId = _swapChain->getSelectedId(_commandPool, x, y);
            std::cout << "Selected: " << selectedId << std::endl;
            
            if (selectedId >= 256) {
                _scene->draughts()->move(Draught::Position{static_cast<int8_t>((selectedId - 256) % 10),
                                                           static_cast<int8_t>((selectedId - 256) / 10)});
            } else {
                for (auto&& draught : _scene->draughts()->draughts()) {
                    if (draught.state() == Draught::State::Crown) {
                        draught.setSelected(draught.id() == selectedId || draught.lady()->id() == selectedId);
                    } else if (draught.state() == Draught::State::Lady) {
                        draught.setSelected(draught.id() == selectedId || draught.crown()->id() == selectedId);
                    } else {
                        draught.setSelected(draught.id() == selectedId);
                    }
                }
            }
        });
    }
    
    ~HelloTriangleApplication()
    {
        
    }
    
    void run()
    {
        _window->run([this]() {
            this->drawFrame();
        });
        vkDeviceWaitIdle(*_device);
    }

private:
    void recreateSwapChain()
    {
        _window->waitForRestore();
        vkDeviceWaitIdle(*_device);

        _swapChain = std::make_shared<SwapChain>(_device, _surface, _commandPool, _descriptorSetLayout, _scene, _window->framebufferSize());
    }

    void drawFrame()
    {
        vkWaitForFences(*_device, 1, &_syncObjects->inFlightFence(_frameManager->current()), VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(*_device, *_swapChain, UINT64_MAX, _syncObjects->imageAvailableSemaphore(_frameManager->current()), VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        
        // Move sphere
        static float spherePos = 0.6;
        static int sphereDirection = 1;
        
        _scene->sphere()->setPosition(0, glm::vec3(0.0, spherePos, 0.9));
        _scene->sphere()->setPosition(1, glm::vec3(0.0, -spherePos, 0.9));
        _scene->sphere()->setPosition(2, glm::vec3(spherePos, 0.0, 0.9));
        _scene->sphere()->setPosition(3, glm::vec3(-spherePos, 0.0, 0.9));
        
        if (_moving) {
            if (sphereDirection > 0) {
                spherePos += 0.01;
                
                if (spherePos > 1) {
                    sphereDirection = -1;
                }
            } else if (sphereDirection < 0) {
                spherePos -= 0.01;
                
                if (spherePos < -1) {
                    sphereDirection = 1;
                }
            }
        }
        
        _window->handleInput();
        _swapChain->updateUniformBuffer(_camera, imageIndex);
        _swapChain->updateLightingBuffer(_camera, glm::vec3(0.0, 0.1, 1.8), imageIndex, _glassAlgo);
        _scene->updateInstanceBufferObjects(imageIndex);
        
        if (_syncObjects->imageInFlight(imageIndex) != VK_NULL_HANDLE) {
            vkWaitForFences(*_device, 1, &_syncObjects->imageInFlight(imageIndex), VK_TRUE, UINT64_MAX);
        }
        _syncObjects->imageInFlight(imageIndex) = _syncObjects->inFlightFence(_frameManager->current());

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {_syncObjects->imageAvailableSemaphore(_frameManager->current())};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_swapChain->commandBuffers()->get(imageIndex);

        VkSemaphore signalSemaphores[] = {_syncObjects->renderFinishedSemaphore(_frameManager->current())};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(*_device, 1, &_syncObjects->inFlightFence(_frameManager->current()));

        if (vkQueueSubmit(_device->graphicsQueue(), 1, &submitInfo, _syncObjects->inFlightFence(_frameManager->current())) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        
        static auto last = std::chrono::high_resolution_clock::now();
        static auto now  = std::chrono::high_resolution_clock::now();
        static auto fps = 0;
        fps++;
        
        now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()) > std::chrono::duration_cast<std::chrono::seconds>(last.time_since_epoch())) {
            last = now;
            std::cout << "FPS: " << fps << std::endl;
            fps = 0;
        }
        
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {*_swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(_device->presentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _window->readAndResetWindowResizedFlag()) {
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        _frameManager->increment();
    }
    
private:
    std::shared_ptr<Window> _window;
    
    std::shared_ptr<Instance> _instance;
    std::shared_ptr<Surface> _surface;
    std::shared_ptr<Device> _device;
    std::shared_ptr<CommandPool> _commandPool;
    std::shared_ptr<Scene> _scene;
    std::shared_ptr<DescriptorSetLayout> _descriptorSetLayout;
    
    std::shared_ptr<TextureImage> _textureImage;
    
    std::shared_ptr<SwapChain> _swapChain;
    
    std::shared_ptr<SyncObjects> _syncObjects;
    std::shared_ptr<FrameManager> _frameManager;
    
    std::shared_ptr<Camera> _camera;
    
    uint32_t _glassAlgo{0};
    bool _moving{false};
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
