#version 450

layout(binding = 0) uniform sampler2D uTextureSampler;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 color;

void main() {
	vec4 tex = texture(uTextureSampler, uv).rgba;

	//if (tex.a < 0.5) discard;

	color = vec4(tex.rgb, 1);
}