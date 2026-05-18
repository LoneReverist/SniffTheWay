// Vertex.ixx

module;

#include <concepts>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

export module Vertex;

import Dreamhearth;
using namespace Dreamhearth;

export template<typename VertexT>
LayoutDesc create_layout()
{
    LayoutDesc layout;
	layout.binding = 0; // vertex uses binding 0, instance uses binding 1
	layout.stride = sizeof(VertexT);
	layout.input_rate = InputRate::Vertex;

	std::uint32_t location = 0;

	// All vertex types must have a position attribute.
	AttributeType pos_type = AttributeType::Float;
	if constexpr (std::same_as<glm::vec3, decltype(VertexT::pos)>)
		pos_type = AttributeType::Float3;
	else if constexpr (std::same_as<glm::vec2, decltype(VertexT::pos)>)
		pos_type = AttributeType::Float2;
	else
		static_assert(!std::same_as<VertexT, VertexT>, "Unsupported position format in VertexT");

	layout.attributes.push_back(AttributeDesc{
		.type = pos_type,
		.offset = offsetof(VertexT, pos),
		.location = location++
		});

	if constexpr (requires(VertexT v) { v.normal; })
	{
		layout.attributes.push_back(AttributeDesc{
			.type = AttributeType::Float3,
			.offset = offsetof(VertexT, normal),
			.location = location++
			});
	}

	if constexpr (requires(VertexT v) { v.tex_coord; })
	{
		layout.attributes.push_back(AttributeDesc{
			.type = AttributeType::Float2,
			.offset = offsetof(VertexT, tex_coord),
			.location = location++
			});
	}

	if constexpr (requires(VertexT v) { v.color; })
	{
		layout.attributes.push_back(AttributeDesc{
			.type = AttributeType::Float3,
			.offset = offsetof(VertexT, color),
			.location = location++
			});
	}

	return layout;
}

export struct Vertex2d
{
	glm::vec2 pos{ 0.0f };

	static LayoutDesc CreateLayout() { return create_layout<Vertex2d>(); }
};

export struct TextureVertex2d
{
	glm::vec2 pos{ 0.0f };
	glm::vec2 tex_coord{ 0.0f };

	static LayoutDesc CreateLayout() { return create_layout<TextureVertex2d>(); }
};

export template <typename T>
concept IsVertex =
	std::same_as<T, Vertex2d>
	|| std::same_as<T, TextureVertex2d>;
