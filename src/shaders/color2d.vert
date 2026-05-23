#version 420 core

layout(std140, binding = 0) uniform ProjUniform {
	mat4 proj;
} transforms;

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 out_color;

void main()
{
	out_color = in_color;

	gl_Position = transforms.proj * vec4(in_pos, 0.0, 1.0);
}
