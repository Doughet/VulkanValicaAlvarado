#version 450
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in int inTextIndex;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out flat int outTextIndex;

void main() {
    fragTexCoord = inTexCoord;
    outTextIndex = inTextIndex;
    gl_Position = vec4(inPosition, 0.0, 1.0);
}
