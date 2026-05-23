// Vertex.ixx

module;

#include <concepts>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

export module Vertex;

import Dreamhearth;
namespace dh = Dreamhearth;

export template<typename VertexT>
dh::LayoutDesc create_layout()
{
    dh::LayoutDesc layout;
	layout.binding = 0; // vertex uses binding 0, instance uses binding 1
	layout.stride = sizeof(VertexT);
	layout.input_rate = dh::InputRate::Vertex;

	std::uint32_t location = 0;

	// All vertex types must have a position attribute.
	dh::AttributeType pos_type = dh::AttributeType::Float;
	if constexpr (std::same_as<glm::vec3, decltype(VertexT::pos)>)
		pos_type = dh::AttributeType::Float3;
	else if constexpr (std::same_as<glm::vec2, decltype(VertexT::pos)>)
		pos_type = dh::AttributeType::Float2;
	else
		static_assert(!std::same_as<VertexT, VertexT>, "Unsupported position format in VertexT");

	layout.attributes.push_back(dh::AttributeDesc{
		.type = pos_type,
		.offset = offsetof(VertexT, pos),
		.location = location++
		});

	if constexpr (requires(VertexT v) { v.normal; })
	{
		layout.attributes.push_back(dh::AttributeDesc{
			.type = dh::AttributeType::Float3,
			.offset = offsetof(VertexT, normal),
			.location = location++
			});
	}

	if constexpr (requires(VertexT v) { v.tex_coord; })
	{
		layout.attributes.push_back(dh::AttributeDesc{
			.type = dh::AttributeType::Float2,
			.offset = offsetof(VertexT, tex_coord),
			.location = location++
			});
	}

	if constexpr (requires(VertexT v) { v.color; })
	{
		dh::AttributeType pos_type = dh::AttributeType::Float3;
		if constexpr (std::same_as<glm::vec4, decltype(VertexT::color)>)
			pos_type = dh::AttributeType::Float4;

		layout.attributes.push_back(dh::AttributeDesc{
			.type = pos_type,
			.offset = offsetof(VertexT, color),
			.location = location++
			});
	}

	return layout;
}

export struct Vertex2d
{
	glm::vec2 pos{ 0.0f };

	static dh::LayoutDesc CreateLayout() { return create_layout<Vertex2d>(); }
};

export struct TextureVertex2d
{
	glm::vec2 pos{ 0.0f };
	glm::vec2 tex_coord{ 0.0f };

	static dh::LayoutDesc CreateLayout() { return create_layout<TextureVertex2d>(); }
};

export struct ColorVertex2d
{
	glm::vec2 pos{ 0.0f };
	glm::vec4 color{ 1.0f };

	static dh::LayoutDesc CreateLayout() { return create_layout<ColorVertex2d>(); }
};

export template <typename T>
concept IsVertex =
	std::same_as<T, Vertex2d>
	|| std::same_as<T, TextureVertex2d>
	|| std::same_as<T, ColorVertex2d>;
