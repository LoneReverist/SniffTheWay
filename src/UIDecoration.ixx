// UIDecoration.ixx

module;

#include <expected>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>

export module UIDecoration;

import Dreamhearth;

import AssetManager;
import AssetPool;
import DecorationAtlas;
import Texture2dPipeline;
import Vertex;

namespace dh = Dreamhearth;

export class UIDecoration
{
public:
	using VertexT = TextureVertex2d;

	struct Bounds
	{
		glm::vec2 min{ 0.0f };
		glm::vec2 max{ 0.0f };
		bool is_valid = false;

		glm::vec2 Size() const { return is_valid ? max - min : glm::vec2{ 0.0f }; }
	};

public:
	UIDecoration() = default;

	void Init(
		AssetManager & asset_manager,
		AssetId texture_id,
		SniffTheWay::Decorations::DecorationId decoration_id,
		glm::vec2 center,
		float scale,
		glm::vec4 color = glm::vec4{ 1.0f });

	void SetDecorationId(SniffTheWay::Decorations::DecorationId decoration_id);
	void SetCenter(glm::vec2 center);
	void SetScale(float scale);
	void SetColor(glm::vec4 color);

	void SetROId(AssetId ro_id) { m_ro_id = ro_id; }

	MeshId<VertexT> GetMeshId() const { return m_mesh_id; }
	AssetId GetROId() const { return m_ro_id; }
	Texture2dPipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }
	Bounds const & GetBounds() const { return m_bounds; }
	SniffTheWay::Decorations::DecorationInfo const & GetDecorationInfo() const;

private:
	std::expected<dh::Mesh, dh::GraphicsError> create_mesh() const;
	void update_mesh() const;

private:
	AssetManager * m_asset_manager = nullptr;
	MeshId<VertexT> m_mesh_id;
	AssetId m_ro_id;

	SniffTheWay::Decorations::DecorationId m_decoration_id = SniffTheWay::Decorations::DecorationId::HorizontalDividerPawFlourish;
	glm::vec2 m_center{ 0.0f };
	float m_scale = 1.0f;
	Texture2dPipeline::ObjectData m_pipeline_data;
	mutable Bounds m_bounds;
};

void UIDecoration::Init(
	AssetManager & asset_manager,
	AssetId texture_id,
	SniffTheWay::Decorations::DecorationId decoration_id,
	glm::vec2 center,
	float scale,
	glm::vec4 color)
{
	m_asset_manager = &asset_manager;
	m_decoration_id = decoration_id;
	m_center = center;
	m_scale = scale;
	m_pipeline_data = Texture2dPipeline::ObjectData{
		.tex_id = texture_id,
		.color = color,
		.color_mode = Texture2dPipeline::ColorMode::ColorMask,
	};

	std::expected<dh::Mesh, dh::GraphicsError> mesh = create_mesh();
	if (!mesh.has_value())
	{
		std::cout << "UIDecoration::Init: Failed to create mesh. Error: " << mesh.error().GetMessage() << std::endl;
		return;
	}

	m_mesh_id = m_asset_manager->AddMesh<VertexT>(std::move(mesh.value()));
}

void UIDecoration::SetDecorationId(SniffTheWay::Decorations::DecorationId decoration_id)
{
	if (m_decoration_id == decoration_id)
		return;

	m_decoration_id = decoration_id;
	update_mesh();
}

void UIDecoration::SetCenter(glm::vec2 center)
{
	if (m_center == center)
		return;

	m_center = center;
	update_mesh();
}

void UIDecoration::SetScale(float scale)
{
	if (m_scale == scale)
		return;

	m_scale = scale;
	update_mesh();
}

void UIDecoration::SetColor(glm::vec4 color)
{
	m_pipeline_data.color = color;
}

SniffTheWay::Decorations::DecorationInfo const & UIDecoration::GetDecorationInfo() const
{
	return SniffTheWay::Decorations::Get(m_decoration_id);
}

std::expected<dh::Mesh, dh::GraphicsError> UIDecoration::create_mesh() const
{
	if (!m_asset_manager)
		return std::unexpected{ dh::GraphicsError{ "UIDecoration::create_mesh called before Init." } };

	SniffTheWay::Decorations::DecorationInfo const & decoration = GetDecorationInfo();
	SniffTheWay::Decorations::PixelBounds const & bounds = decoration.bounds;

	const float draw_width = static_cast<float>(bounds.width) * m_scale;
	const float draw_height = static_cast<float>(bounds.height) * m_scale;
	const float left = m_center.x - draw_width * 0.5f;
	const float top = m_center.y - draw_height * 0.5f;
	const float right = left + draw_width;
	const float bottom = top + draw_height;

	const float min_u = static_cast<float>(bounds.x) / SniffTheWay::Decorations::TextureWidth;
	const float max_u = static_cast<float>(bounds.x + bounds.width) / SniffTheWay::Decorations::TextureWidth;
	const float min_v = static_cast<float>(bounds.y) / SniffTheWay::Decorations::TextureHeight;
	const float max_v = static_cast<float>(bounds.y + bounds.height) / SniffTheWay::Decorations::TextureHeight;

	std::vector<VertexT> verts{
		{ { left, top }, { min_u, min_v } },
		{ { right, top }, { max_u, min_v } },
		{ { left, bottom }, { min_u, max_v } },
		{ { right, bottom }, { max_u, max_v } } };

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	m_bounds = Bounds{
		.min = glm::vec2{ left, top },
		.max = glm::vec2{ right, bottom },
		.is_valid = true,
	};

	dh::Mesh mesh{ m_asset_manager->GetRenderContext() };
	std::expected<void, dh::GraphicsError> result = mesh.Create(verts, indices);
	if (!result.has_value())
		return std::unexpected{ result.error().AddToMessage(" UIDecoration::create_mesh: Failed to create mesh.") };

	return mesh;
}

void UIDecoration::update_mesh() const
{
	std::expected<dh::Mesh, dh::GraphicsError> new_mesh = create_mesh();
	if (!new_mesh.has_value())
	{
		std::cout << "UIDecoration::update_mesh: Failed to create mesh: " << new_mesh.error().GetMessage() << std::endl;
		return;
	}

	if (m_asset_manager)
		m_asset_manager->UpdateMesh(m_mesh_id, std::move(new_mesh.value()));
}
