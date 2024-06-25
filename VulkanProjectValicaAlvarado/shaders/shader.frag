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
layout(location = 7) in flat int currentSelected; // The camera position in view space

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform LightBufferObject{
    vec3 lightDir;
    vec3 lightPos;
    vec3 viewPos;
    vec3 lightColor;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    float lightIntensity;
} ubo2;

vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 Md, vec3 Ms, float gamma) {
    vec3 Diffuse = Md * clamp(dot(N, L),0.0,1.0);
    vec3 Specular = Ms * vec3(pow(clamp(dot(N, normalize(V + L)),0.0,1.0), gamma));
    return (Diffuse + Specular);
}

void Blinn(){
    vec3 Norm = normalize(fragNormal);
    vec3 EyeDir = normalize(ubo2.viewPos - fragPos);
    vec3 Albedo = texture(texSamplerArray[outIndex], fragTexCoord).rgb;
    float pex = 200.0f;
    float metallic = 1.0f;

    vec3 lightColor = ubo2.lightColor.rgb;
    vec3 DiffSpec = vec3(0.0f);
    vec3 L = normalize(ubo2.lightDir);
    DiffSpec += BRDF(EyeDir, Norm, L, Albedo, vec3(metallic), pex);



    // A special type of non-uniform ambient color, invented for this course
    const vec3 cxp = vec3(1.0,0.5,0.5) * 0.15;
    const vec3 cxn = vec3(0.9,0.6,0.4) * 0.15;
    const vec3 cyp = vec3(0.3,1.0,1.0) * 0.15;
    const vec3 cyn = vec3(0.5,0.5,0.5) * 0.15;
    const vec3 czp = vec3(0.8,0.2,0.4) * 0.15;
    const vec3 czn = vec3(0.3,0.6,0.7) * 0.15;

    vec3 Ambient =((Norm.x > 0 ? cxp : cxn) * (Norm.x * Norm.x) +
    (Norm.y > 0 ? cyp : cyn) * (Norm.y * Norm.y) +
    (Norm.z > 0 ? czp : czn) * (Norm.z * Norm.z)) * Albedo;

    vec3 result = DiffSpec * lightColor.rgb * clamp(ubo2.lightIntensity, 0.0, 1.0);

    vec4 res4 = vec4(clamp(result, 0.0f, 1.0f), 1.0f);

    outColor = res4 * float(outIndex != currentSelected) + vec4(0.0f, 1.0f, 0.0f, 1.0f) * float(outIndex == currentSelected);
}

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

void BlinnPhong(){ // Just do Blinn. This function has a couple of problems
    vec3 Norm = normalize(fragNormal);
    vec3 EyeDir = normalize(ubo2.viewPos - fragPos);
    vec3 Albedo = texture(texSamplerArray[outIndex], fragTexCoord).rgb;
    float pex = 200.0f;
    float metallic = 1.0f;
    vec3 L = ubo2.lightDir;
    vec3 lightColor = ubo2.lightColor.rgb;

    vec3 DiffSpec = BRDF(EyeDir, Norm, L, Albedo, vec3(metallic), pex);

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
    Blinn();
    //lambertModel();

    /*
        if(inIndex == 4) {
            outColor = vec4(1.0, 1.0, 1.0, 1.0);
        }else{
            outColor = vec4(texture(texSamplerArray[inIndex], fragTexCoord).rgb, 1.0);
        }
    */
}