#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform LightBufferObject {
    vec3 lightPos;
    vec3 viewPos;
    vec3 lightColor;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
} ubo2;


void main() {
    vec4 colorBeforeLight = texture(texSampler, fragTexCoord);



    // Simulate ambient lighting effect
    float distance = length(ubo2.lightPos - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 ambient = 0.2 * vec3(1.0, 1.0, 1.0) * attenuation;

    // Combine texture color with simulated ambient lighting
    vec3 result = colorBeforeLight.rgb * ambient;
    outColor = vec4(result, colorBeforeLight.a);

}