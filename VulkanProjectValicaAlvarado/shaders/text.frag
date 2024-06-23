#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat int textIndex;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler[2];

void main() {
    outColor = texture(texSampler[textIndex], fragTexCoord);
}