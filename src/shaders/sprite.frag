#version 420 core

#ifndef BUILD_VULKAN
#extension GL_KHR_vulkan_glsl : enable
#endif

layout(set = 1, binding = 0) uniform sampler2D tex_sampler;

#ifdef BUILD_VULKAN
layout(push_constant) uniform ObjectData {
	layout(offset = 64) vec4 frame_uvs; // Frame UVs: x = min_u, y = max_u, z = min_v, w = max_v
	layout(offset = 80) vec4 tint;
} obj_data;

#else // OpenGL
layout(std140, binding = 9) uniform ObjectDataFS {
	vec4 frame_uvs; // Frame UVs: x = min_u, y = max_u, z = min_v, w = max_v
	vec4 tint;
} obj_data;

#endif

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	// Map normalized UV (0-1) to the frame's UV coordinates
	float u = mix(obj_data.frame_uvs.x, obj_data.frame_uvs.y, in_uv.x);
	float v = mix(obj_data.frame_uvs.z, obj_data.frame_uvs.w, in_uv.y);
	
	vec4 tex_color = texture(tex_sampler, vec2(u, v));
	if (tex_color.a == 0.0)
		discard;

	out_frag_color = tex_color * obj_data.tint;
}
