#version 420 core

#ifndef BUILD_VULKAN
#extension GL_KHR_vulkan_glsl : enable
#endif

layout(set = 1, binding = 0) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	out_frag_color = texture(tex_sampler, in_uv);
}
