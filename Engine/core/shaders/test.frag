#version 450

layout(binding = 0) uniform readonly UniformInput {
    mat4 model;
    mat4 invModel;
    mat4 view;
    mat4 proj;
    uvec2 screenSize;
} uniformInput;

layout(r8ui, binding = 1) uniform readonly uimage3D pixels;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 modelPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 proportion = (gl_FragCoord.xy / uniformInput.screenSize - 0.5) * 2;

    vec3 viewRay = vec3(proportion.xy, 1);
    normalize(viewRay);

    vec3 modelRay = (uniformInput.invModel * vec4(viewRay, 0)).xyz;

    uint index = imageLoad(pixels, ivec3(floor((modelPos.x + 1) * 64), floor((modelPos.y + 1) * 64), 0)).r;

    outColor = vec4(index, 0, 0, 1);
}