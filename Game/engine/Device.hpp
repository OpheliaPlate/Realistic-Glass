#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <optional>
#include <vector>

class Instance;
class Surface;

class Device
{
public:
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
        
public:
    Device(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface);
    ~Device();
    
    operator VkDevice();
    VkPhysicalDevice physicalDevice();
    VkQueue graphicsQueue();
    VkQueue presentQueue();
    VkSampleCountFlagBits msaaSamples();
    
public:
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, std::shared_ptr<Surface> surface);
    static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, std::shared_ptr<Surface> surface);
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
