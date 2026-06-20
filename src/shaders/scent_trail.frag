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
	float elapsed_time;
	float glow_speed;
	float glow_width;
	float glow_intensity;
} obj_data;

#else // OpenGL
layout(std140, binding = 9) uniform ObjectDataFS {
	vec4 color;
	vec2 dog_pos;
	float visible_distance;
	float base_opacity;
	float elapsed_time;
	float glow_speed;
	float glow_width;
	float glow_intensity;
} obj_data;

#endif

layout(location = 0) in vec3 in_world_pos;
layout(location = 1) in float in_trail_t;
layout(location = 2) in float in_side;

layout(location = 0) out vec4 out_frag_color;

float hash21(vec2 p)
{
	return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

vec2 hash22(vec2 p)
{
	return vec2(
		hash21(p + vec2(17.0, 53.0)),
		hash21(p + vec2(71.0, 29.0)));
}

float sparkle_generation_mask(vec2 cell, vec2 local, float generation, float elapsed_time)
{
	const float sparkle_density = 0.62;
	const float sparkle_radius = 0.29;
	const float twinkle_speed = 8.5;

	vec2 generation_key = cell + vec2(generation * 19.37, generation * 47.11);
	float seed = hash21(generation_key);
	float enabled = step(1.0 - sparkle_density, seed);
	vec2 sparkle_offset = (hash22(generation_key) - 0.5) * 0.72;
	vec2 sparkle_pos = local - sparkle_offset;

	float dist = length(sparkle_pos);
	float core = 1.0 - smoothstep(0.0, sparkle_radius * 0.32, dist);
	float halo = 1.0 - smoothstep(sparkle_radius * 0.16, sparkle_radius, dist);
	float horizontal_ray = (1.0 - smoothstep(0.0, 0.035, abs(sparkle_pos.y)))
		* (1.0 - smoothstep(0.07, 0.44, abs(sparkle_pos.x)));
	float vertical_ray = (1.0 - smoothstep(0.0, 0.035, abs(sparkle_pos.x)))
		* (1.0 - smoothstep(0.07, 0.44, abs(sparkle_pos.y)));
	float star = max(max(core, halo * 0.36), max(horizontal_ray, vertical_ray) * 0.55);

	float phase = hash21(generation_key + vec2(91.0, 13.0)) * 6.2831853;
	float twinkle = sin(elapsed_time * twinkle_speed + phase) * 0.5 + 0.5;
	twinkle = smoothstep(0.18, 1.0, twinkle);

	return star * twinkle * enabled;
}

float sparkle_mask(float trail_t, float side, float elapsed_time, float directional_glow, float center_glow)
{
	const float cells_along_trail = 52.0;
	const float cells_across_trail = 3.0;
	const float generation_speed = 2.4;

	vec2 sparkle_uv = vec2(
		trail_t * cells_along_trail,
		(side * 0.5 + 0.5) * cells_across_trail);
	vec2 cell = floor(sparkle_uv);
	vec2 local = fract(sparkle_uv) - 0.5;

	float generation_time = elapsed_time * generation_speed;
	float generation = floor(generation_time);
	float generation_blend = smoothstep(0.15, 0.85, fract(generation_time));
	float previous = sparkle_generation_mask(cell, local, generation, elapsed_time);
	float next = sparkle_generation_mask(cell, local, generation + 1.0, elapsed_time);
	float sparkle = mix(previous, next, generation_blend);

	float band_presence = 0.25 + directional_glow * 0.9;

	return sparkle * band_presence * center_glow;
}

void main()
{
	vec2 dog_pos = obj_data.dog_pos;
	float visible_distance = max(obj_data.visible_distance, 0.001);
	float base_opacity = obj_data.base_opacity;
	float glow_width = max(obj_data.glow_width, 0.001);

	float edge_fade = 1.0 - smoothstep(0.35, 1.0, abs(in_side));
	float dog_distance = distance(in_world_pos.xy, dog_pos);
	float dog_fade = 1.0 - smoothstep(visible_distance * 0.15, visible_distance, dog_distance);

	float center_glow = 1.0 - smoothstep(0.0, 0.75, abs(in_side));
	float glow_center = fract(obj_data.elapsed_time * obj_data.glow_speed);
	float glow_distance = (in_trail_t - glow_center) / glow_width;
	float directional_glow = exp(-glow_distance * glow_distance) * center_glow;
	float sparkles = sparkle_mask(in_trail_t, in_side, obj_data.elapsed_time, directional_glow, center_glow);
	float alpha = obj_data.color.a * base_opacity * edge_fade * dog_fade;
	alpha *= 1.0 + directional_glow * obj_data.glow_intensity * 0.45;
	alpha += sparkles * dog_fade * 1.0;

	if (alpha < 0.004)
		discard;

	vec3 color = obj_data.color.rgb * (1.0
		+ center_glow * 0.35
		+ directional_glow * obj_data.glow_intensity);
	color += vec3(0.72, 0.92, 1.0) * sparkles * dog_fade * 4.2;
	color += vec3(1.0, 1.0, 0.86) * sparkles * dog_fade * 1.6;
	out_frag_color = vec4(color, alpha);
}
