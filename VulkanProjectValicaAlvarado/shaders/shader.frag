#version 450

layout(set = 0, binding = 4) uniform sampler2D texSamplerArray[10];

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in flat int inIndex;

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
    // Base color for the object is white
    vec3 baseColor = vec3(1.0, 1.0, 1.0);

    // Normalize the fragment normal
    vec3 norm = normalize(fragNormal);

    // Calculate the direction of the light
    vec3 lightDir = normalize(ubo2.lightPos - fragPos);

    // Calculate the ambient component
    vec3 ambient = ubo2.ambientColor * baseColor;

    // Calculate the diffuse component using Lambert's cosine law
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = ubo2.diffuseColor * diff * baseColor;

    // Combine the components for the final color
    vec3 finalColor = ambient + diffuse;

    // Ensure the color does not exceed white
    finalColor = clamp(finalColor, 0.0, 1.0);


    outColor = vec4(texture(texSamplerArray[inIndex], fragTexCoord).rgb, 1.0);

/*
    if(inIndex == 3) {
        outColor = vec4(1.0, 1.0, 1.0, 1.0);
  s  }else{
        outColor = vec4(texture(texSamplerArray[inIndex], fragTexCoord).rgb, 1.0);
    }
*/

}
