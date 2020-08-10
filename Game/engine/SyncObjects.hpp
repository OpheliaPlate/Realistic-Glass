#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class Device;

class SyncObjects
{
public:
    SyncObjects(std::shared_ptr<Device> device, size_t amountOfImages);
    ~SyncObjects();
    
    VkSemaphore& imageAvailableSemaphore(size_t index);
    VkSemaphore& renderFinishedSemaphore(size_t index);
    VkFence& inFlightFence(size_t index);
    VkFence& imageInFlight(size_t index);
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
