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

struct MoteField
{
	float core;
	float halo;
	float haze;
};

MoteField add_mote_field(MoteField a, MoteField b)
{
	return MoteField(
		min(a.core + b.core, 1.0),
		min(a.halo + b.halo, 1.0),
		min(a.haze + b.haze, 1.0));
}

MoteField mote_generation(vec2 cell, vec2 local, float generation, float elapsed_time, float lane_center_bias)
{
	const float edge_density = 0.34;
	const float center_density = 0.96;
	const float core_radius = 0.06;
	const float halo_radius = 0.21;
	const float haze_radius = 0.42;
	const float twinkle_speed = 5.8;

	vec2 generation_key = cell + vec2(generation * 19.37, generation * 47.11);
	float seed = hash21(generation_key);
	float mote_density = mix(edge_density, center_density, lane_center_bias);
	float enabled = step(1.0 - mote_density, seed);
	vec2 mote_offset = (hash22(generation_key) - 0.5) * 0.86;
	vec2 mote_pos = local - mote_offset;

	float dist = length(mote_pos);
	float core = 1.0 - smoothstep(core_radius * 0.2, core_radius, dist);
	float halo = 1.0 - smoothstep(core_radius, halo_radius, dist);
	float haze = 1.0 - smoothstep(halo_radius * 0.55, haze_radius, dist);
	float horizontal_ray = (1.0 - smoothstep(0.0, 0.018, abs(mote_pos.y)))
		* (1.0 - smoothstep(core_radius, halo_radius * 1.25, abs(mote_pos.x)));
	float vertical_ray = (1.0 - smoothstep(0.0, 0.018, abs(mote_pos.x)))
		* (1.0 - smoothstep(core_radius, halo_radius * 1.25, abs(mote_pos.y)));

	float phase = hash21(generation_key + vec2(91.0, 13.0)) * 6.2831853;
	float twinkle = sin(elapsed_time * twinkle_speed + phase) * 0.5 + 0.5;
	twinkle = mix(0.42, 1.0, smoothstep(0.18, 1.0, twinkle));

	return MoteField(
		max(core, max(horizontal_ray, vertical_ray) * 0.72) * twinkle * enabled * mix(0.45, 1.0, lane_center_bias),
		halo * twinkle * enabled * mix(0.45, 1.0, lane_center_bias),
		haze * enabled * mix(0.35, 1.0, lane_center_bias));
}

MoteField mote_field(float trail_t, float side, float elapsed_time, float directional_glow, float center_glow)
{
	const float cells_along_trail = 170.0;
	const float cells_across_trail = 7.0;
	const float generation_speed = 1.55;

	vec2 mote_uv = vec2(
		trail_t * cells_along_trail,
		(side * 0.5 + 0.5) * cells_across_trail);
	vec2 cell = floor(mote_uv);
	vec2 local = fract(mote_uv) - 0.5;

	float generation_time = elapsed_time * generation_speed;
	float generation = floor(generation_time);
	float generation_blend = smoothstep(0.15, 0.85, fract(generation_time));
	MoteField field = MoteField(0.0, 0.0, 0.0);

	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
			vec2 neighbor = vec2(float(x), float(y));
			vec2 neighbor_cell = cell + neighbor;
			vec2 neighbor_local = local - neighbor;
			float lane_side = ((neighbor_cell.y + 0.5) / cells_across_trail) * 2.0 - 1.0;
			float lane_center_bias = 1.0 - smoothstep(0.05, 0.82, abs(lane_side));
			MoteField previous = mote_generation(neighbor_cell, neighbor_local, generation, elapsed_time, lane_center_bias);
			MoteField next = mote_generation(neighbor_cell, neighbor_local, generation + 1.0, elapsed_time, lane_center_bias);
			field = add_mote_field(field, MoteField(
				mix(previous.core, next.core, generation_blend),
				mix(previous.halo, next.halo, generation_blend),
				mix(previous.haze, next.haze, generation_blend)));
		}
	}

	float band_presence = 0.55 + directional_glow * 0.65;
	float center_bias = mix(0.22, 1.45, center_glow);
	field.core *= band_presence * center_bias;
	field.halo *= band_presence * center_bias;
	field.haze *= (0.7 + directional_glow * 0.35) * center_bias;
	return field;
}

void main()
{
	vec2 dog_pos = obj_data.dog_pos;
	float visible_distance = max(obj_data.visible_distance, 0.001);
	float base_opacity = obj_data.base_opacity;
	float glow_width = max(obj_data.glow_width, 0.001);

	float edge_fade = 1.0 - smoothstep(0.35, 1.0, abs(in_side));
	float soft_edge = 1.0 - smoothstep(0.72, 1.0, abs(in_side));
	float dog_distance = distance(in_world_pos.xy, dog_pos);
	float dog_fade = 1.0 - smoothstep(visible_distance * 0.15, visible_distance, dog_distance);

	float center_glow = 1.0 - smoothstep(0.0, 0.75, abs(in_side));
	float center_spine = exp(-in_side * in_side * 18.0);
	float center_haze = exp(-in_side * in_side * 4.5);
	
	float pulse_margin = glow_width * 2.2;
	float pulse_phase = fract(obj_data.elapsed_time * obj_data.glow_speed);
	float glow_front = pulse_phase * (1.0 + pulse_margin * 2.0) - pulse_margin;
	float front_delta = in_trail_t - glow_front;
	float behind_front = max(-front_delta, 0.0);
	float ahead_of_front = max(front_delta, 0.0);
	float leading_edge = 1.0 - smoothstep(0.0, glow_width * 0.28, ahead_of_front);
	float trailing_core = exp(-pow(behind_front / (glow_width * 0.78), 2.0));
	float trailing_halo = exp(-pow(behind_front / (glow_width * 1.85), 2.0));
	float directional_profile = leading_edge * trailing_core;
	float halo_profile = (1.0 - smoothstep(0.0, glow_width * 0.48, ahead_of_front)) * trailing_halo;
	float directional_glow = directional_profile * max(center_glow, center_spine);
	float pulse_core = directional_profile * (0.65 + center_spine * 1.35) * obj_data.glow_intensity;
	float pulse_halo = halo_profile * center_haze * obj_data.glow_intensity;
	MoteField motes = mote_field(in_trail_t, in_side, obj_data.elapsed_time, directional_glow, center_glow);
	float aura = edge_fade * (0.2 + center_haze * 0.65 + center_spine * 0.85 + pulse_halo * 0.1);
	float alpha = dog_fade * (
		obj_data.color.a * base_opacity * aura * 0.55
		+ center_spine * 0.35
		+ pulse_halo * 0.05
		+ pulse_core * 0.07
		+ motes.haze * 0.42
		+ motes.halo * 1.08
		+ motes.core * 1.95);

	if (alpha < 0.004)
		discard;

	vec3 aura_color = vec3(1.0, 0.47, 0.12);
	vec3 halo_color = vec3(1.0, 0.63, 0.16);
	vec3 mote_color = vec3(1.0, 0.94, 0.34);
	vec3 core_color = vec3(1.0, 0.99, 0.76);

	vec3 color =
		aura_color * aura * soft_edge * 0.95
		+ halo_color * center_haze * 0.9
		+ mote_color * center_spine * 1.25
		+ halo_color * pulse_halo * 0.26
		+ core_color * pulse_core * 0.34
		+ halo_color * motes.haze * 1.35
		+ mote_color * motes.halo * 3.05
		+ core_color * motes.core * 8.5;
	color *= dog_fade;
	out_frag_color = vec4(color, alpha);
}
