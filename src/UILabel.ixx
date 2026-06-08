// UILabel.ixx

module;

#include <expected>
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

export module UILabel;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import AssetPool;
import FontAtlas;
import TextPipeline;
import Vertex;

export class UILabel
{
public:
	using VertexT = TextureVertex2d;

	enum class Align {
		Left,
		Right,
		Center,
	};

	struct Bounds
	{
		glm::vec2 min{ 0.0f };
		glm::vec2 max{ 0.0f };
		bool is_valid = false;

		glm::vec2 Size() const { return is_valid ? max - min : glm::vec2{ 0.0f }; }
	};

public:
	explicit UILabel(
		AssetManager & asset_manager,
		std::string_view text,
		FontAtlas const & font_atlas,
		float font_size,
		glm::vec2 origin,
		Align align,
		glm::vec4 color);

	void SetText(std::string_view text);
	void SetFontSize(float font_size);
	void SetOrigin(glm::vec2 origin);
	void SetAlign(Align align);

	void SetROId(AssetId ro_id) { m_ro_id = ro_id; }
	
	MeshId<VertexT> GetMeshId() const { return m_mesh_id; }
	AssetId GetROId() const { return m_ro_id; }
    TextPipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }
	Bounds const & GetBounds() const { return m_bounds; }

private:
	std::expected<dh::Mesh, dh::GraphicsError> create_mesh() const;
	void update_mesh() const;

private:
	AssetManager & m_asset_manager;
	MeshId<VertexT> m_mesh_id;
	AssetId m_ro_id;

	std::string m_text;
	FontAtlas const & m_font_atlas;
	std::uint32_t m_font_tex_width = 0;
	std::uint32_t m_font_tex_height = 0;
	float m_font_size = 0.0f;
	glm::vec2 m_origin{ 0.0f };
	Align m_align = Align::Left;
	TextPipeline::ObjectData m_pipeline_data;
	mutable Bounds m_bounds;
};

UILabel::UILabel(
	AssetManager & asset_manager,
	std::string_view text,
	FontAtlas const & font_atlas,
	float font_size,
	glm::vec2 origin,
	Align align,
	glm::vec4 color)
	: m_asset_manager(asset_manager)
	, m_text(text)
	, m_font_atlas(font_atlas)
	, m_font_size(font_size)
	, m_origin(origin)
	, m_align(align)
{
	std::uint32_t font_tex_width = 0, font_tex_height = 0;
	dh::Texture const * font_tex = m_asset_manager.GetTexture(font_atlas.GetTextureId());
	if (font_tex)
	{
		m_font_tex_width = font_tex->GetWidth();
		m_font_tex_height = font_tex->GetHeight();
	}

	m_pipeline_data = TextPipeline::ObjectData{
		.screen_px_range = font_size * font_atlas.GetPxRange(),
		.bg_color = { 0.0f, 0.0f, 0.0f, 0.0f },
		.text_color = color,
		.tex_id = font_atlas.GetTextureId(),
	};

	std::expected<dh::Mesh, dh::GraphicsError> mesh = create_mesh();
	if (!mesh.has_value())
	{
		std::cout << "UILabel::UILabel: Failed to create mesh. Error: " << mesh.error().GetMessage() << std::endl;
		return;
	}

	m_mesh_id = m_asset_manager.AddMesh<VertexT>(std::move(mesh.value()));
}

void UILabel::SetText(std::string_view text)
{
	if (m_text == text)
		return; // no change

	m_text = text;

	update_mesh();
}

void UILabel::SetFontSize(float font_size)
{
	if (m_font_size == font_size)
		return; // no change

	m_font_size = font_size;
	m_pipeline_data.screen_px_range = m_font_size * m_font_atlas.GetPxRange();

	update_mesh();
}

void UILabel::SetOrigin(glm::vec2 origin)
{
	if (m_origin == origin)
		return; // no change

	m_origin = origin;

	update_mesh();
}

void UILabel::SetAlign(Align align)
{
	if (m_align == align)
		return; // no change

	m_align = align;

	update_mesh();
}

std::expected<dh::Mesh, dh::GraphicsError> UILabel::create_mesh() const
{
	if (m_font_tex_width == 0 || m_font_tex_height == 0 || m_text.empty())
	{
		m_bounds = Bounds{};
		return dh::Mesh{ m_asset_manager.GetRenderContext() }; // an empty mesh is an expected result here
	}

	std::vector<VertexT> verts;
	std::vector<dh::Mesh::IndexT> indices;

	glm::vec2 pen = m_origin;
	for (char c : m_text)
	{
		auto glyph = m_font_atlas.GetGlyph(static_cast<std::uint32_t>(c));
		if (!glyph.has_value())
			continue; // skip missing glyphs

		FontAtlas::Glyph const & g = glyph.value();

		// For the space character we just advance the pen position
		if (!g.plane_bounds.has_value() || !g.atlas_bounds.has_value())
		{
			pen.x += g.advance * m_font_size;
			continue;
		}

		// left, bottom, right, top
		glm::vec4 pb = g.plane_bounds.value();
		pb.x *= m_font_size;
		pb.z *= m_font_size;
		pb.y *= m_font_size;
		pb.w *= m_font_size;
		glm::vec4 uv = g.atlas_bounds.value();
		uv.x /= m_font_tex_width;
		uv.z /= m_font_tex_width;
		uv.y /= m_font_tex_height;
		uv.w /= m_font_tex_height;

		// 2 triangles
		dh::Mesh::IndexT start_vi = verts.size();
		verts.push_back({ { pen.x + pb.x, pen.y - pb.w }, { uv.x, uv.w } }); // top-left
		verts.push_back({ { pen.x + pb.z, pen.y - pb.w }, { uv.z, uv.w } }); // top-right
		verts.push_back({ { pen.x + pb.x, pen.y - pb.y }, { uv.x, uv.y } }); // bottom-left
		verts.push_back({ { pen.x + pb.z, pen.y - pb.y }, { uv.z, uv.y } }); // bottom-right

		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 1));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 0));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 2));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 1));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 2));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 3));

		pen.x += g.advance * m_font_size;
	}

	// alignment
	float x_offset = 0.0f;
	if (m_align == Align::Right)
		x_offset = m_origin.x - pen.x;
	else if (m_align == Align::Center)
		x_offset = (m_origin.x - pen.x) * 0.5;

	if (x_offset != 0.0f)
	{
		for (VertexT & vert : verts)
			vert.pos.x += x_offset;
	}

	if (verts.empty())
	{
		m_bounds = Bounds{};
	}
	else
	{
		glm::vec2 min{
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max()
		};
		glm::vec2 max{
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest()
		};
		for (VertexT const & vert : verts)
		{
			min.x = std::min(min.x, vert.pos.x);
			min.y = std::min(min.y, vert.pos.y);
			max.x = std::max(max.x, vert.pos.x);
			max.y = std::max(max.y, vert.pos.y);
		}
		m_bounds = Bounds{
			.min = min,
			.max = max,
			.is_valid = true
		};
	}

	dh::Mesh mesh{ m_asset_manager.GetRenderContext() };
	std::expected<void, dh::GraphicsError> result = mesh.Create(verts, indices);
	if (!result.has_value())
		return std::unexpected{ result.error().AddToMessage(" UILabel::CreateMesh: Failed to create mesh.") };

	return mesh;
}

void UILabel::update_mesh() const
{
	std::expected<dh::Mesh, dh::GraphicsError> new_mesh = create_mesh();
	if (!new_mesh.has_value())
	{
		std::cout << "UILabel::update_mesh: Failed to create mesh: " << new_mesh.error().GetMessage() << std::endl;
		return;
	}

	m_asset_manager.UpdateMesh(m_mesh_id, std::move(new_mesh.value()));
}
