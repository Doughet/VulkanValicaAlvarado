#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    int currentSelected;
} ubo; // Camera

layout(binding = 3) uniform MatrixBufferObject {
    mat4 model[20]; // Matrix containing all the models
} mubo;

layout(location = 0) in vec3 inPosition; // Vertex position
layout(location = 1) in vec3 inColor; // The vertex color
layout(location = 2) in vec2 inTexCoord; // The texture coordinates
layout(location = 3) in vec3 inNormal; // Normal Vector
layout(location = 4) in int inIndex; // Index to select  which model matrix to use from 'mubo'
layout(location = 5) in int inHasNormal; // Flag indicating if the vertex has a normal
layout(location = 6) in vec3 inTangent; // Tangent vector (not used in the shader but declared)
layout(location = 7) in vec3 inBita; // Bitangent vector (not used in the shader but declared)

layout(location = 0) out vec3 fragPos; // The transformed position in the view space
layout(location = 1) out vec2 fragTexCoord; // The texture coordinates
layout(location = 2) out vec3 fragNormal; // The transformed normal vector
layout(location = 3) out vec3 fragColor; // The vertex color
layout(location = 4) out int outIndex; // The model index
layout(location = 5) out flat int outHasNormal; // The flag indicating if the vertex has a normal
layout(location = 6) out vec3 outCamPos; // The camera position in view space
layout(location = 7) out flat int currentSelected; // The camera position in view space


void main() {

    gl_Position = ubo.proj * ubo.view * ubo.model * mubo.model[inIndex] * vec4(inPosition, 1.0);

    fragPos = vec3(ubo.model * mubo.model[inIndex] * vec4(inPosition, 1.0));
    fragTexCoord = inTexCoord;

    mat3 normalMatrix = transpose(inverse(mat3(mubo.model[inIndex])));

    currentSelected = ubo.currentSelected;

    fragNormal = normalize(normalMatrix * inNormal);
    fragColor = inColor;
    outIndex = inIndex;
    outHasNormal = inHasNormal;
    outCamPos = vec3(inverse(ubo.view)[3]); // Inverts the camera view

}