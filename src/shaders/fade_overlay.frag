#version 420 core

#ifndef BUILD_VULKAN
#extension GL_KHR_vulkan_glsl : enable
#endif

#ifdef BUILD_VULKAN
layout(push_constant) uniform ObjectData {
	vec4 color;
} obj_data;

#else // OpenGL
layout(std140, binding = 9) uniform ObjectDataFS {
	vec4 color;
} obj_data;

#endif

layout(location = 0) in vec4 in_color;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	out_frag_color = obj_data.color * in_color;
}

