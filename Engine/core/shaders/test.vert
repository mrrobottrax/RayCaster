#version 450

layout(location = 0) in vec3 pos;
layout(location = 0) out vec3 fragColor;

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(pos, 1.0);
    fragColor = colors[gl_VertexIndex % 3];
}