#version 450

layout(binding = 1) uniform LightingBufferObject {
    vec4 viewPosition;
    vec4 lightPosition;
    vec4 lightColor;
} lbo;

layout(location = 0) in vec3 fragPosition;
layout(location = 4) flat in int fragTexIndex;

layout (location = 0) out float outFragColor;

void main()
{
    // Store distance to light as 32 bit float value
    vec3 lightVec = fragPosition.xyz - vec3(lbo.lightPosition);
    outFragColor = length(lightVec);
}
