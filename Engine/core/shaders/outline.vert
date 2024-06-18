#version 450

layout(binding = 0) uniform readonly OutlineInput {
	mat4 view;
	ivec3 blockPos;
} uInput;

vec3 positions[2] = vec3[](
    vec3(-1, -1, 0),
    vec3( 1,  1, 0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex % 2], 1);
}