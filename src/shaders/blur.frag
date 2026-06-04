#version 420 core

#ifndef BUILD_VULKAN
#extension GL_KHR_vulkan_glsl : enable
#endif

layout(set = 1, binding = 0) uniform sampler2D tex_sampler;

#ifdef BUILD_VULKAN
layout(push_constant) uniform ObjectData {
	vec2 texel_step;
	int blur_radius;
	float alpha_boost;
} obj_data;

#else // OpenGL
layout(std140, binding = 9) uniform ObjectDataFS {
	vec2 texel_step;
	int blur_radius;
	float alpha_boost;
} obj_data;

#endif

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	int radius = clamp(obj_data.blur_radius, 0, 64);
	float sigma = max(float(radius) * 0.5, 1.0);
	float two_sigma_sq = 2.0 * sigma * sigma;
	float alpha = 0.0;
	float total_weight = 0.0;

	for (int i = -radius; i <= radius; ++i)
	{
		float offset = float(i);
		float weight = exp(-(offset * offset) / two_sigma_sq);
		alpha += texture(tex_sampler, in_uv + obj_data.texel_step * offset).a * weight;
		total_weight += weight;
	}

	if (total_weight > 0.0)
		alpha /= total_weight;

	out_frag_color = vec4(1.0, 1.0, 1.0, min(alpha * obj_data.alpha_boost, 1.0));
}
