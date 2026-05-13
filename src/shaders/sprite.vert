#version 420 core

layout(std140, binding = 0) uniform ViewProjUniform {
	mat4 view;
	mat4 proj;
} transforms;

#ifdef BUILD_VULKAN
layout(push_constant) uniform ObjectData {
	mat4 model;
} obj_data;

#else // OpenGL
layout(std140, binding = 8) uniform ObjectDataVS {
	mat4 model;
} obj_data;

#endif

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec2 out_uv;

void main()
{
	out_uv = in_uv;

	vec4 pos_world = obj_data.model * vec4(in_pos.x, in_pos.y, 0.0, 1.0);
	gl_Position = transforms.proj * transforms.view * pos_world;
}
