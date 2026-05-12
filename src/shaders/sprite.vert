#version 420 core

layout(std140, binding = 0) uniform ViewProjUniform {
	mat4 view;
	mat4 proj;
} transforms;

#ifdef BUILD_VULKAN
layout(push_constant) uniform ObjectData {
	vec3 position;
} obj_data;

#else // OpenGL
layout(std140, binding = 8) uniform ObjectDataFS {
	vec3 position;
} obj_data;

#endif

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec2 out_uv;

void main()
{
	out_uv = in_uv;

	vec3 pos_world = vec3(in_pos.x, 0.0, in_pos.y) + obj_data.position;
	gl_Position = transforms.proj * transforms.view * vec4(pos_world, 1.0);
}
