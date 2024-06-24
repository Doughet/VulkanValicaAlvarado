#version 450

layout(set = 0, binding = 0) uniform sampler2D texSamplerArray;

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(texture(texSamplerArray, inPosition).rgb, 1.0f);
}
