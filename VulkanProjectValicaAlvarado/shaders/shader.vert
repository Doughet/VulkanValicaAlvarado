#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 3) uniform MatrixBufferObject {
    mat4 model[20];
} mubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inIndex;
layout(location = 5) in int inHasNormal;
layout(location = 6) in vec3 inTangent;
layout(location = 7) in vec3 inBita;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragColor;
layout(location = 4) out int outIndex;
layout(location = 5) out flat int outHasNormal;
layout(location = 6) out vec3 outCamPos;


void main() {

    gl_Position = ubo.proj * ubo.view * ubo.model * mubo.model[inIndex] * vec4(inPosition, 1.0);

    fragPos = vec3(ubo.model * mubo.model[inIndex] * vec4(inPosition, 1.0));
    fragTexCoord = inTexCoord;

    mat3 normalMatrix = transpose(inverse(mat3(mubo.model[inIndex])));

    fragNormal = normalize(normalMatrix * inNormal);
    fragColor = inColor;
    outIndex = inIndex;
    outHasNormal = inHasNormal;

    outCamPos = vec3(inverse(ubo.view)[3]);

}