#version 450

layout(location = 0) in mat4 matrix;
layout(location = 0) out vec2 uv;

vec3 positions[6] = vec3[](
	vec3(-1, -1, 0),
	vec3(1, 1, 0),
	vec3(-1, 1, 0),
	vec3(1, 1, 0),
	vec3(-1, -1, 0),
	vec3(1, -1, 0)
);

vec2 uvs[6] = vec2[](
	vec2(0, 0),
	vec2(1, 1),
	vec2(0, 1),
	vec2(1, 1),
	vec2(0, 0),
	vec2(1, 0)
);

void main() {
    gl_Position = matrix * vec4(positions[gl_VertexIndex % 6], 1);

	uv = uvs[gl_VertexIndex % 6];
}