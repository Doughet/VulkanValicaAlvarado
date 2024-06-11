#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 3) uniform MatrixBufferObject {
    mat4 model[5];
} mubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inIndex;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragColor;


void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * mubo.model[inIndex] * vec4(inPosition, 1.0);
    fragPos = vec3(ubo.model * mubo.model[inIndex] * vec4(inPosition, 1.0));
    fragTexCoord = inTexCoord;
    fragNormal = mat3(ubo.model) * inNormal;
    fragColor = inColor;
}