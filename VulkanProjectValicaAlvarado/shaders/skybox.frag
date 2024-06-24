#version 450

layout(set = 0, binding = 0) uniform sampler2D texSamplerArray;

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform TimeBuffer {
    float time;
}tbo;

vec3 oscillateColor(float t, vec3 color1, vec3 color2) {
    float factor = (sin(t) * 0.5) + 0.5; // Oscillates between 0 and 1
    return mix(color1, color2, factor);
}

void main() {
    vec3 color1 = vec3(234.0 / 255.0, 215.0 / 255.0, 209.0 / 255.0);
    vec3 color2 = vec3(234.0 / 255.0, 187.0 / 255.0, 168.0 / 255.0);

    vec3 color = oscillateColor(tbo.time, color1, color2);

    outColor = vec4(color, 1.0);
}
