#version 420 core

#ifndef BUILD_VULKAN
#extension GL_KHR_vulkan_glsl : enable
#endif

layout(set = 1, binding = 0) uniform sampler2D tex_sampler;

#ifdef BUILD_VULKAN
layout(push_constant) uniform ObjectData {
	vec4 color;
	int color_mode;
} obj_data;

#else // OpenGL
layout(std140, binding = 9) uniform ObjectDataFS {
	vec4 color;
	int color_mode;
} obj_data;

#endif

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	vec4 tex_color = texture(tex_sampler, in_uv);

	if (obj_data.color_mode == 1)
	{
		out_frag_color = vec4(obj_data.color.rgb, obj_data.color.a * tex_color.a);
		return;
	}

	out_frag_color = tex_color * obj_data.color;
}
