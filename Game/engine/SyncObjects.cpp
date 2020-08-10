#include "SyncObjects.hpp"

#include <exception>

#include "Device.hpp"
#include "FrameManager.hpp"

class SyncObjects::Private
{
public:
    Private(std::shared_ptr<Device> device, size_t amountOfImages)
        : _device(device)
    {
        _imageAvailableSemaphores.resize(FrameManager::MAX_FRAMES_IN_FLIGHT);
        _renderFinishedSemaphores.resize(FrameManager::MAX_FRAMES_IN_FLIGHT);
        _inFlightFences.resize(FrameManager::MAX_FRAMES_IN_FLIGHT);
        _imagesInFlight.resize(amountOfImages, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < FrameManager::MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(*_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(*_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(*_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }
    
    ~Private()
    {
        for (size_t i = 0; i < FrameManager::MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(*_device, _renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(*_device, _imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(*_device, _inFlightFences[i], nullptr);
        }
    }
    
    VkSemaphore& imageAvailableSemaphore(size_t index)
    {
        return _imageAvailableSemaphores.at(index);
    }
    
    VkSemaphore& renderFinishedSemaphore(size_t index)
    {
        return _renderFinishedSemaphores.at(index);
    }
    
    VkFence& inFlightFence(size_t index)
    {
        return _inFlightFences.at(index);
    }
    
    VkFence& imageInFlight(size_t index)
    {
        return _imagesInFlight.at(index);
    }
    
private:
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;
    
    std::shared_ptr<Device> _device;
};

SyncObjects::SyncObjects(std::shared_ptr<Device> device, size_t amountOfImages)
    : _delegate(std::make_unique<Private>(device, amountOfImages))
{
    
}

SyncObjects::~SyncObjects()
{
    
}

VkSemaphore& SyncObjects::imageAvailableSemaphore(size_t index)
{
    return _delegate->imageAvailableSemaphore(index);
}

VkSemaphore& SyncObjects::renderFinishedSemaphore(size_t index)
{
    return _delegate->renderFinishedSemaphore(index);
}

VkFence& SyncObjects::inFlightFence(size_t index)
{
    return _delegate->inFlightFence(index);
}

VkFence& SyncObjects::imageInFlight(size_t index)
{
    return _delegate->imageInFlight(index);
}
