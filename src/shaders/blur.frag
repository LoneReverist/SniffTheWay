#version 420 core

#ifndef BUILD_VULKAN
#extension GL_KHR_vulkan_glsl : enable
#endif

layout(set = 1, binding = 0) uniform sampler2D tex_sampler;

#ifdef BUILD_VULKAN
layout(push_constant) uniform ObjectData {
	vec2 texel_step;
} obj_data;

#else // OpenGL
layout(std140, binding = 9) uniform ObjectDataFS {
	vec2 texel_step;
} obj_data;

#endif

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	float alpha = 0.0;
	alpha += texture(tex_sampler, in_uv - obj_data.texel_step * 4.0).a * 0.0162162162;
	alpha += texture(tex_sampler, in_uv - obj_data.texel_step * 3.0).a * 0.0540540541;
	alpha += texture(tex_sampler, in_uv - obj_data.texel_step * 2.0).a * 0.1216216216;
	alpha += texture(tex_sampler, in_uv - obj_data.texel_step).a * 0.1945945946;
	alpha += texture(tex_sampler, in_uv).a * 0.2270270270;
	alpha += texture(tex_sampler, in_uv + obj_data.texel_step).a * 0.1945945946;
	alpha += texture(tex_sampler, in_uv + obj_data.texel_step * 2.0).a * 0.1216216216;
	alpha += texture(tex_sampler, in_uv + obj_data.texel_step * 3.0).a * 0.0540540541;
	alpha += texture(tex_sampler, in_uv + obj_data.texel_step * 4.0).a * 0.0162162162;

	out_frag_color = vec4(1.0, 1.0, 1.0, alpha);
}
