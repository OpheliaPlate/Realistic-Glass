#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 5) in vec3 instancePos;

layout(location = 0) out vec3 fragPosition;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 eye;
    mat4 referencePoints[4];
} ubo;

layout(push_constant) uniform PushConsts {
    mat4 model;
    mat4 view;
    mat4 proj;
    int  referencePointIndex;
} pushConsts;
 
out gl_PerVertex
{
    vec4 gl_Position;
};
 
void main()
{
    gl_Position = pushConsts.proj * pushConsts.view * ubo.referencePoints[pushConsts.referencePointIndex] * vec4(inPosition + instancePos, 1.0);
    fragPosition = inPosition + instancePos;
}
