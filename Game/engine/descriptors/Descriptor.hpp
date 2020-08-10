#pragma once

#include <vulkan/vulkan.h>

struct Descriptor
{
    uint32_t binding;
    VkDescriptorType descriptorType;
    VkShaderStageFlagBits stageFlags;
};
