#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

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
    vec3 texColor = texture(texSampler, fragTexCoord).rgb;

    //vec3 texColor = vec3(1.0, 1.0, 1.0);

    // Ambient lighting
    vec3 ambient = ubo2.ambientColor * texColor;

    // Diffuse lighting
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(vec3(0.0, 0.5, 0.0) - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = vec3(0.0, 1.0, 0.0) * diff * texColor * 3.0;

    // Specular lighting
    vec3 viewDir = normalize(ubo2.viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), ubo2.shininess);
    vec3 specular = vec3(0.0, 0.0, 1.0) * spec * texColor;

    // Combine results
    vec3 result = ambient + diffuse ;
    outColor = vec4(result, 1.0);

}