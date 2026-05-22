// UILabel.ixx

module;

#include <expected>
#include <functional>
#include <iostream>
#include <string>
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

public:
	explicit UILabel(
		AssetManager & asset_manager,
		std::string const & text,
		FontAtlas const & font_atlas,
		float font_size,
		glm::vec2 origin,
		Align align,
		glm::vec4 color);

	void OnViewportResized(int width, int height);

	void SetText(std::string const & text);
	void SetFontSize(float font_size);

	MeshId<VertexT> GetMeshId() const { return m_mesh_id; }
    TextPipeline::ObjectData const & GetLabelData() const { return m_label_data; }

private:
	std::expected<dh::Mesh, dh::GraphicsError> create_mesh() const;
	void update_mesh() const;

private:
	AssetManager & m_asset_manager;
	MeshId<VertexT> m_mesh_id;

	std::string m_text;
	FontAtlas const & m_font_atlas;
	std::uint32_t m_font_tex_width = 0;
	std::uint32_t m_font_tex_height = 0;
	float m_font_size = 0.0f;
	glm::vec2 m_origin{ 0.0f };
	Align m_align = Align::Left;
	TextPipeline::ObjectData m_label_data;

	int m_viewport_width = 0;
	int m_viewport_height = 0;
};

UILabel::UILabel(
	AssetManager & asset_manager,
	std::string const & text,
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

	m_label_data = TextPipeline::ObjectData{
		.screen_px_range = font_size * font_atlas.GetPxRange(),
		.bg_color = { 0.0f, 0.0f, 0.0f, 0.0f },
		.text_color = color,
	};

	std::expected<dh::Mesh, dh::GraphicsError> mesh = create_mesh();
	if (!mesh.has_value())
	{
		std::cout << "UILabel::UILabel: Failed to create mesh. Error: " << mesh.error().GetMessage() << std::endl;
		return;
	}

	m_mesh_id = m_asset_manager.AddMesh<VertexT>(std::move(mesh.value()));
}

void UILabel::OnViewportResized(int width, int height)
{
	if (width == m_viewport_width && height == m_viewport_height)
		return; // no change

	m_viewport_width = width;
	m_viewport_height = height;

	update_mesh();
}

void UILabel::SetText(std::string const & text)
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
	m_label_data.screen_px_range = m_font_size * m_font_atlas.GetPxRange();

	update_mesh();
}

std::expected<dh::Mesh, dh::GraphicsError> UILabel::create_mesh() const
{
	if (m_font_tex_width == 0 || m_font_tex_height == 0
		|| m_viewport_width == 0 || m_viewport_height == 0
		|| m_text.empty())
	{
		return dh::Mesh{ m_asset_manager.GetRenderContext() }; // an empty mesh is an expected result here
	}

	std::vector<VertexT> verts;
	std::vector<dh::Mesh::IndexT> indices;

	// convert font size to screen coordinates -1 to 1
	float height_scale = m_font_size * (2.0f / m_viewport_height);
	float width_scale = m_font_size * (2.0f / m_viewport_width);

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
			pen.x += g.advance * width_scale;
			continue;
		}

		// left, bottom, right, top
		glm::vec4 pb = g.plane_bounds.value();
		pb.x *= width_scale;
		pb.z *= width_scale;
		pb.y *= height_scale;
		pb.w *= height_scale;
		glm::vec4 uv = g.atlas_bounds.value();
		uv.x /= m_font_tex_width;
		uv.z /= m_font_tex_width;
		uv.y /= m_font_tex_height;
		uv.w /= m_font_tex_height;

		// 2 triangles
		dh::Mesh::IndexT start_vi = verts.size();
		verts.push_back({ { pen.x + pb.x, pen.y + pb.w }, { uv.x, uv.w } }); // top-left
		verts.push_back({ { pen.x + pb.z, pen.y + pb.w }, { uv.z, uv.w } }); // top-right
		verts.push_back({ { pen.x + pb.x, pen.y + pb.y }, { uv.x, uv.y } }); // bottom-left
		verts.push_back({ { pen.x + pb.z, pen.y + pb.y }, { uv.z, uv.y } }); // bottom-right

		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 1));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 0));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 2));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 1));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 2));
		indices.push_back(static_cast<dh::Mesh::IndexT>(start_vi + 3));

		pen.x += g.advance * width_scale;
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
