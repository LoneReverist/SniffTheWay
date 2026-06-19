#version 420 core

#ifndef BUILD_VULKAN
#extension GL_KHR_vulkan_glsl : enable
#endif

#ifdef BUILD_VULKAN
layout(push_constant) uniform ObjectData {
	vec4 color;
	vec2 dog_pos;
	float visible_distance;
	float base_opacity;
} obj_data;

#else // OpenGL
layout(std140, binding = 9) uniform ObjectDataFS {
	vec4 color;
	vec2 dog_pos;
	float visible_distance;
	float base_opacity;
} obj_data;

#endif

layout(location = 0) in vec3 in_world_pos;
layout(location = 1) in float in_trail_t;
layout(location = 2) in float in_side;

layout(location = 0) out vec4 out_frag_color;

float sparkle(float trail_t)
{
	float a = sin(trail_t * 157.0) * 43758.5453;
	float b = sin(trail_t * 61.0 + 3.1) * 24634.6345;
	return fract(a + b);
}

void main()
{
	vec2 dog_pos = obj_data.dog_pos;
	float visible_distance = max(obj_data.visible_distance, 0.001);
	float base_opacity = obj_data.base_opacity;

	float edge_fade = 1.0 - smoothstep(0.35, 1.0, abs(in_side));
	float dog_distance = distance(in_world_pos.xy, dog_pos);
	float dog_fade = 1.0 - smoothstep(visible_distance * 0.15, visible_distance, dog_distance);

	float center_glow = 1.0 - smoothstep(0.0, 0.75, abs(in_side));
	float glint = smoothstep(0.965, 1.0, sparkle(floor(in_trail_t * 80.0) / 80.0));
	float alpha = obj_data.color.a * base_opacity * edge_fade * dog_fade;

	if (alpha < 0.004)
		discard;

	vec3 color = obj_data.color.rgb * (1.0 + center_glow * 0.35 + glint * 0.25);
	out_frag_color = vec4(color, alpha);
}
