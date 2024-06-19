#version 450

layout(binding = 0) uniform readonly OutlineInput {
	mat4 view;
	mat4 perspective;
	ivec3 blockPos;
} uInput;

vec3 positions[24] = vec3[](
    // bottom face
	vec3(0, 0, 0),
	vec3(0, 0, 1),

	vec3(0, 0, 1),
	vec3(1, 0, 1),

	vec3(1, 0, 1),
	vec3(1, 0, 0),

	vec3(1, 0, 0),
	vec3(0, 0, 0),

	// top face
	vec3(0, 1, 0),
	vec3(0, 1, 1),

	vec3(0, 1, 1),
	vec3(1, 1, 1),

	vec3(1, 1, 1),
	vec3(1, 1, 0),

	vec3(1, 1, 0),
	vec3(0, 1, 0),

	// vertical edges
	vec3(0, 0, 0),
	vec3(0, 1, 0),

	vec3(0, 0, 1),
	vec3(0, 1, 1),

	vec3(1, 0, 1),
	vec3(1, 1, 1),

	vec3(1, 0, 0),
	vec3(1, 1, 0)
);

layout(location = 0) out vec4 color;

void main() {
    gl_Position = uInput.perspective * uInput.view * vec4(positions[gl_VertexIndex % 24] + uInput.blockPos, 1);
}