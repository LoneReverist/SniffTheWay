#version 420 core

#ifndef BUILD_VULKAN
#extension GL_KHR_vulkan_glsl : enable
#endif

layout(set = 1, binding = 0) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	float alpha = texture(tex_sampler, in_uv).a;
	out_frag_color = vec4(1.0, 1.0, 1.0, alpha);
}
