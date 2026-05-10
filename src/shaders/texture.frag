#version 450

layout(std140, binding = 1) uniform LightsUniform {
	vec3 ambient_light_color;
} lights;

layout(binding = 2) uniform sampler2D tex_sampler;

layout(location = 0) in vec3 in_pos_world;
layout(location = 1) in vec3 in_normal_world;
layout(location = 2) in vec2 in_tex_coord;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	vec3 normal = normalize(in_normal_world);

	vec3 light_color = lights.ambient_light_color;

	out_frag_color = texture(tex_sampler, in_tex_coord) * vec4(light_color, 1.0);
}
