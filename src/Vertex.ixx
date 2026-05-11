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
	layout.stride = sizeof(VertexT);

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

export struct Texture2dVertex
{
	glm::vec2 pos;
	glm::vec2 tex_coord;

	static LayoutDesc CreateLayout() { return create_layout<Texture2dVertex>(); }
};

export template <typename T>
concept IsVertex =
	std::same_as<T, Texture2dVertex>;
