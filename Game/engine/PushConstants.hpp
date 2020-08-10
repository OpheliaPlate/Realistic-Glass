#pragma once

#include <glm/glm.hpp>

struct PushConstants
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(4)  uint32_t referencePointIndex;
};
