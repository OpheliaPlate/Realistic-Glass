#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 2) uniform samplerCube shadowCubeMap;

layout(binding = 3) uniform sampler2D texSampler1;
layout(binding = 4) uniform sampler2D texSampler2;
layout(binding = 5) uniform sampler2D texSampler3;
layout(binding = 6) uniform sampler2D texSampler4;
layout(binding = 7) uniform sampler2D texSampler5;
layout(binding = 8) uniform sampler2D texSampler6;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) flat in int fragId;
layout(location = 5) flat in int fragTexIndex;
layout(location = 6) flat in int fragSelected;
layout(location = 7) flat in int fragInstanceIndex;

layout(location = 0) out vec4 outColor;

#define EPSILON 0.15
#define SHADOW_OPACITY 0.3

void main()
{
    int selectedId = fragId;
    if (selectedId == 5) {
        int x = int(((fragPosition.x + 0.15) * 100.0)) / 3;
        int y = int(((fragPosition.y + 0.15) * 100.0)) / 3;
        selectedId = 256 + y * 10 + x;
    }
    outColor = vec4(selectedId, 0.0, 0.0, 0.0);
}
