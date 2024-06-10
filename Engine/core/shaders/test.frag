#version 450

layout(binding = 0) uniform readonly UniformInput {
    mat4 model;
    mat4 view;
    mat4 proj;
    uvec2 screenSize;
} uniformInput;

layout(r8ui, binding = 1) uniform readonly uimage3D pixels;

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    uvec4 index = imageLoad(pixels, ivec3(0, 0, 0));
    
    vec2 proportion = gl_FragCoord.xy / uniformInput.screenSize;

    outColor = vec4(proportion, 0, 1);
}