#version 450

layout(set = 0, binding = 4) uniform sampler2D texSamplerArray[10];
layout(set = 0, binding = 5) uniform sampler2D normalSamplerArray[10];

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in flat int outIndex;
layout(location = 5) in flat int outHasNormal;
layout(location = 6) in vec3 outCamPos;

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

void lambertModel(){
    //vec3 lightDirection = normalize(ubo2.lightPos - fragPos);

    vec3 normal = normalize(fragNormal);


    vec3 lightDirection1 = vec3(0.0f, 0.0, 1.0f);
    vec3 lightDirection2 = vec3(0.0f, 1.0, 0.0f);
    vec3 lightDirection3 = vec3(0.0f, -1.0, 0.0f);
    vec3 lightDirection4 = vec3(1.0f, 0.0, 0.0f);

    vec3 Diffuse1 = texture(texSamplerArray[outIndex], fragTexCoord).rgb
    * clamp(dot(normal, lightDirection1),0.0,1.0) * 1;

    vec3 Diffuse2 = texture(texSamplerArray[outIndex], fragTexCoord).rgb
    * clamp(dot(normal, lightDirection2),0.0,1.0) * 1;

    vec3 Diffuse3 = texture(texSamplerArray[outIndex], fragTexCoord).rgb
    * clamp(dot(normal, lightDirection3),0.0,1.0) * 1;

    vec3 Diffuse4 = texture(texSamplerArray[outIndex], fragTexCoord).rgb
    * clamp(dot(normal, lightDirection4),0.0,1.0) * 1;


    vec3 Ambient = texture(texSamplerArray[outIndex], fragTexCoord).rgb * ubo2.ambientColor * 0.1;

    //vec3 Diffuse = texture(texSamplerArray[outIndex], fragTexCoord).rgb
    //* 1;
    outColor = vec4(Diffuse1 + Diffuse2 + Diffuse3 + Diffuse4 + Ambient, 1.0);
}

void BlinnPhong(){
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(ubo2.lightPos - fragPos);
    vec3 viewDir = normalize(outCamPos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // Ambient
    vec3 ambient = ubo2.ambientColor;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = ubo2.diffuseColor * diff;

    // Specular
    float spec = pow(max(dot(norm, halfwayDir), 0.0), ubo2.shininess);
    vec3 specular = ubo2.specularColor * spec;

    // Combine results
    vec3 result = diffuse ;
    outColor = vec4(texture(texSamplerArray[outIndex], fragTexCoord).rgb * result * ubo2.lightColor, 1.0);
}


void main() {
    //BlinnPhong();
    lambertModel();

    /*
        if(inIndex == 4) {
            outColor = vec4(1.0, 1.0, 1.0, 1.0);
        }else{
            outColor = vec4(texture(texSamplerArray[inIndex], fragTexCoord).rgb, 1.0);
        }
    */
}