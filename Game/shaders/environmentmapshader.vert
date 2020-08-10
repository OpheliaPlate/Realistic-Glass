#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 eye;
    mat4 referencePoints[5];
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

// Instanced attributes
layout(location = 4) in int instanceId;
layout(location = 5) in vec3 instancePos;
layout(location = 6) in vec3 instanceRot;
layout(location = 7) in float instanceScale;
layout(location = 8) in int instanceTexIndex;
layout(location = 9) in int instanceSelected;
layout(location = 10) in float instanceFresnel;
layout(location = 11) in vec3 instanceColor;
layout(location = 12) in float instanceRefraction;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) out vec3 fragInstancePos;
layout(location = 5) out int fragTexIndex;
layout(location = 6) out int fragSelected;
layout(location = 7) out float fragFresnel;
layout(location = 8) out int fragInstanceIndex;
layout(location = 9) out vec3 fragInstanceColor;
layout(location = 10) out int fragInstanceReferencePointIndex;
layout(location = 11) out vec3 fragInstanceReferencePoint;
layout(location = 12) out float fragInstanceRefraction;
layout(location = 13) out vec3 referencePoint;

layout(push_constant) uniform PushConsts {
    mat4 model;
    mat4 view;
    mat4 proj;
    int  referencePointIndex;
} pushConsts;

void main() {
    gl_Position = pushConsts.proj * pushConsts.view * ubo.referencePoints[pushConsts.referencePointIndex] * vec4(inPosition + instancePos, 1.0);
    fragPosition = inPosition + instancePos;
    fragNormal = inNormal;
    fragColor = inColor;
    fragInstancePos = instancePos;
    fragTexCoord = inTexCoord;
    fragTexIndex = instanceTexIndex;
    fragSelected = instanceSelected;
    fragFresnel = instanceFresnel;
    fragInstanceColor = instanceColor;
    fragInstanceIndex = gl_InstanceIndex;
    fragInstanceReferencePointIndex = fragInstanceIndex+1;
    fragInstanceReferencePoint = vec3(ubo.referencePoints[fragInstanceReferencePointIndex][3]);
    fragInstanceRefraction = instanceRefraction;
    referencePoint = vec3(ubo.referencePoints[pushConsts.referencePointIndex][3]);
}
