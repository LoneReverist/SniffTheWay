#version 420 core

layout(binding = 0) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	out_frag_color = texture(tex_sampler, in_uv);
}
