// UILabel.ixx

module;

#include <expected>
#include <algorithm>
#include <functional>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>
#include <glog/logging.h>

export module UILabel;

import Dreamhearth;

import AssetManager;
import AssetPool;
import FontAtlas;
import TextPipeline;
import Vertex;

namespace dh = Dreamhearth;

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
	UILabel() = default;

	void Init(
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
	void SetTextColor(glm::vec4 color);

	void SetROId(AssetId ro_id) { m_ro_id = ro_id; }
	
	MeshId<VertexT> GetMeshId() const { return m_mesh_id; }
	AssetId GetROId() const { return m_ro_id; }
    TextPipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }
	Bounds const & GetBounds() const { return m_bounds; }
	Bounds GetCharacterBounds(std::size_t index) const
	{
		return index < m_character_bounds.size() ? m_character_bounds[index] : Bounds{};
	}
	std::string_view GetText() const { return m_text; }

private:
	std::expected<dh::Mesh, dh::GraphicsError> create_mesh() const;
	void update_mesh() const;

private:
	AssetManager * m_asset_manager = nullptr;
	MeshId<VertexT> m_mesh_id;
	AssetId m_ro_id;

	std::string m_text;
	FontAtlas const * m_font_atlas = nullptr;
	std::uint32_t m_font_tex_width = 0;
	std::uint32_t m_font_tex_height = 0;
	float m_font_size = 0.0f;
	glm::vec2 m_origin{ 0.0f };
	Align m_align = Align::Left;
	TextPipeline::ObjectData m_pipeline_data;
	mutable Bounds m_bounds;
	mutable std::vector<Bounds> m_character_bounds;
};

void UILabel::Init(
	AssetManager & asset_manager,
	std::string_view text,
	FontAtlas const & font_atlas,
	float font_size,
	glm::vec2 origin,
	Align align,
	glm::vec4 color)
{
	m_asset_manager = &asset_manager;
	m_text = text;
	m_font_atlas = &font_atlas;
	m_font_size = font_size;
	m_origin = origin;
	m_align = align;

	std::uint32_t font_tex_width = 0, font_tex_height = 0;
	dh::Texture const * font_tex = m_asset_manager->GetTexture(font_atlas.GetTextureId());
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
		LOG(ERROR) << "UILabel::UILabel: Failed to create mesh. Error: " << mesh.error().GetMessage();
		return;
	}

	m_mesh_id = m_asset_manager->AddMesh<VertexT>(std::move(mesh.value()));
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
	if (m_font_atlas)
		m_pipeline_data.screen_px_range = m_font_size * m_font_atlas->GetPxRange();

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

void UILabel::SetTextColor(glm::vec4 color)
{
	m_pipeline_data.text_color = color;
}

std::expected<dh::Mesh, dh::GraphicsError> UILabel::create_mesh() const
{
	if (!m_asset_manager || !m_font_atlas)
		return std::unexpected{ dh::GraphicsError{ "UILabel::create_mesh called before Init." } };

	m_character_bounds.assign(m_text.size(), Bounds{});
	if (m_font_tex_width == 0 || m_font_tex_height == 0 || m_text.empty())
	{
		m_bounds = Bounds{};
		return dh::Mesh{ m_asset_manager->GetRenderContext() }; // an empty mesh is an expected result here
	}

	std::vector<VertexT> verts;
	std::vector<dh::Mesh::IndexT> indices;

	glm::vec2 pen = m_origin;
	std::size_t line_start_vi = 0;
	std::size_t line_start_ci = 0;
	const float line_height = m_font_atlas->GetLineHeight() * m_font_size;

	auto align_line = [&](std::size_t start_vi, std::size_t end_vi,
		std::size_t start_ci, std::size_t end_ci, float line_end_x)
	{
		if (start_vi == end_vi)
			return;

		float x_offset = 0.0f;
		if (m_align == Align::Right)
			x_offset = m_origin.x - line_end_x;
		else if (m_align == Align::Center)
			x_offset = (m_origin.x - line_end_x) * 0.5f;

		if (x_offset != 0.0f)
		{
			for (std::size_t i = start_vi; i < end_vi; ++i)
				verts[i].pos.x += x_offset;
			for (std::size_t i = start_ci; i < end_ci; ++i)
			{
				if (!m_character_bounds[i].is_valid)
					continue;
				m_character_bounds[i].min.x += x_offset;
				m_character_bounds[i].max.x += x_offset;
			}
		}
	};

	for (std::size_t character_index = 0; character_index < m_text.size(); ++character_index)
	{
		const char c = m_text[character_index];
		if (c == '\r')
			continue;

		if (c == '\n')
		{
			align_line(line_start_vi, verts.size(), line_start_ci, character_index, pen.x);
			pen.x = m_origin.x;
			pen.y += line_height;
			line_start_vi = verts.size();
			line_start_ci = character_index + 1;
			continue;
		}

		auto glyph = m_font_atlas->GetGlyph(static_cast<std::uint32_t>(c));
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
		m_character_bounds[character_index] = Bounds{
			.min = { pen.x + pb.x, pen.y - pb.w },
			.max = { pen.x + pb.z, pen.y - pb.y },
			.is_valid = true
		};

		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 1));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 0));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 2));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 1));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 2));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 3));

		pen.x += g.advance * m_font_size;
	}
	align_line(line_start_vi, verts.size(), line_start_ci, m_text.size(), pen.x);

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

	dh::Mesh mesh{ m_asset_manager->GetRenderContext() };
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
		LOG(ERROR) << "UILabel::update_mesh: Failed to create mesh: " << new_mesh.error().GetMessage();
		return;
	}

	if (m_asset_manager)
		m_asset_manager->UpdateMesh(m_mesh_id, std::move(new_mesh.value()));
}
