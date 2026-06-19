#version 420 core

layout(std140, binding = 0) uniform ViewProjUniform {
	mat4 view;
	mat4 proj;
} transforms;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in float in_trail_t;
layout(location = 2) in float in_side;

layout(location = 0) out vec3 out_world_pos;
layout(location = 1) out float out_trail_t;
layout(location = 2) out float out_side;

void main()
{
	out_world_pos = in_pos;
	out_trail_t = in_trail_t;
	out_side = in_side;

	gl_Position = transforms.proj * transforms.view * vec4(in_pos, 1.0);
}
