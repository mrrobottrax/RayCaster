#version 450

layout(binding = 0) uniform readonly UniformInput {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 invModel;
    mat4 invView;
    uvec2 screenSize;
} uniformInput;

layout(location = 0) in vec3 pos;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 startPos;

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    startPos = pos;
    gl_Position = uniformInput.proj * uniformInput.view * uniformInput.model * vec4(pos, 1);
    fragColor = colors[gl_VertexIndex % 3];
}