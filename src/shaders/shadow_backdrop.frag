#version 420 core

#ifndef BUILD_VULKAN
#extension GL_KHR_vulkan_glsl : enable
#endif

#ifdef BUILD_VULKAN
layout(push_constant) uniform ObjectData {
	vec2 inner_min_uv;
	vec2 inner_max_uv;
	vec4 color;
} obj_data;

#else // OpenGL
layout(std140, binding = 9) uniform ObjectDataFS {
	vec2 inner_min_uv;
	vec2 inner_max_uv;
	vec4 color;
} obj_data;

#endif

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	vec2 inner_center = (obj_data.inner_min_uv + obj_data.inner_max_uv) * 0.5;
	vec2 inner_half_size = (obj_data.inner_max_uv - obj_data.inner_min_uv) * 0.5;
	vec2 outer_fade_size = max(obj_data.inner_min_uv, vec2(0.0001));

	vec2 normalized_pos = (in_uv - inner_center) / outer_fade_size;
	vec2 normalized_half_size = inner_half_size / outer_fade_size;
	float corner_radius = min(min(normalized_half_size.x, normalized_half_size.y) * 0.75, 1.35);
	vec2 rounded_half_size = max(normalized_half_size - vec2(corner_radius), vec2(0.0));
	vec2 rounded_box_delta = abs(normalized_pos) - rounded_half_size;
	float rounded_box_distance =
		length(max(rounded_box_delta, vec2(0.0)))
		+ min(max(rounded_box_delta.x, rounded_box_delta.y), 0.0)
		- corner_radius;
	float normalized_distance = max(rounded_box_distance, 0.0);
	float alpha = obj_data.color.a * (1.0 - smoothstep(0.0, 1.0, normalized_distance));

	out_frag_color = vec4(obj_data.color.rgb, alpha);
}
