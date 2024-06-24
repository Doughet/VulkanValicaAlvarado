#version 450

layout(set = 0, binding = 0) uniform sampler2D texSamplerArray;

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform TimeBuffer {
    float time;
}tbo;

// Function to convert from hue to RGB using trigonometric functions
vec3 hueToRgb(float hue) {
    float r = sin(hue * 6.2831853); // 2 * PI
    float g = sin(hue * 6.2831853 + 2.0943951); // 2 * PI / 3
    float b = sin(hue * 6.2831853 + 4.1887902); // 4 * PI / 3

    return vec3(r, g, b) * 0.5 + 0.5;
}

void main() {

    // Calculate hue value based on time, wrapping around every 1.0
    float hue = mod(tbo.time * 0.025f, 1.0); // Adjust 0.1 to control speed

    vec3 color = hueToRgb(hue);

    outColor = vec4(color, 1.0);
}
