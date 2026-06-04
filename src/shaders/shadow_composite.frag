#version 420 core

#ifndef BUILD_VULKAN
#extension GL_KHR_vulkan_glsl : enable
#endif

layout(set = 1, binding = 0) uniform sampler2D tex_sampler;

#ifdef BUILD_VULKAN
layout(push_constant) uniform ObjectData {
	vec4 color;
} obj_data;

#else // OpenGL
layout(std140, binding = 9) uniform ObjectDataFS {
	vec4 color;
} obj_data;

#endif

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	float alpha = texture(tex_sampler, in_uv).a;
	out_frag_color = vec4(obj_data.color.rgb, min(obj_data.color.a * alpha, 1.0));
}
