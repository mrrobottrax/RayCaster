#version 450

layout(r8ui, binding = 1) uniform readonly uimage3D pixels;

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}