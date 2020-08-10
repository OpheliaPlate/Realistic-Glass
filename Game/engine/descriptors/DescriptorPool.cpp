#include "DescriptorPool.hpp"

#include <array>
#include <exception>

#include "Device.hpp"
#include "Surface.hpp"

class DescriptorPool::Private
{
public:
    Private(std::shared_ptr<Device> device, const std::vector<Descriptor>& descriptors, size_t amountOfImages)
        : _device(device)
    {
        std::vector<VkDescriptorPoolSize> poolSizes{};
        poolSizes.resize(4*descriptors.size());
        for (int i = 0; i < 4; ++i) {
            for (const auto& descriptor : descriptors) {
                VkDescriptorPoolSize poolSize;
                poolSize.type = descriptor.descriptorType;
                poolSize.descriptorCount = static_cast<uint32_t>(amountOfImages);
                poolSizes.emplace_back(poolSize);
            }
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(amountOfImages) * 4;

        if (vkCreateDescriptorPool(*_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
    
    ~Private()
    {
        vkDestroyDescriptorPool(*_device, _descriptorPool, nullptr);
    }
    
    operator VkDescriptorPool()
    {
        return _descriptorPool;
    }
    
private:
    VkDescriptorPool _descriptorPool;
    
    std::shared_ptr<Device> _device;
};

DescriptorPool::DescriptorPool(std::shared_ptr<Device> device, const std::vector<Descriptor>& descriptors, size_t amountOfImages)
    : _delegate(std::make_unique<Private>(device, descriptors, amountOfImages))
{
    
}

DescriptorPool::~DescriptorPool()
{
    
}

DescriptorPool::operator VkDescriptorPool()
{
    return *_delegate;
}
