// UIImage.ixx

module;

#include <vector>

#include <glm/vec2.hpp>

export module UIImage;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Texture2dPipeline;
import Vertex;

namespace dh = Dreamhearth;

export class UIImage
{
public:
	struct UVBounds
	{
		float min_u = 0.0f;
		float max_u = 1.0f;
		float min_v = 0.0f;
		float max_v = 1.0f;
	};

	struct Bounds
	{
		glm::vec2 min{ 0.0f };
		glm::vec2 max{ 0.0f };

		bool Contains(glm::vec2 point) const
		{
			return point.x >= min.x && point.x <= max.x
				&& point.y >= min.y && point.y <= max.y;
		}
	};

	UIImage() = default;

	void Init(
		AssetManager & asset_manager,
		AssetId texture_id,
		glm::vec2 center,
		glm::vec2 size);
	void Init(
		AssetManager & asset_manager,
		AssetId texture_id,
		glm::vec2 center,
		glm::vec2 size,
		UVBounds uv_bounds);
	void SetUVBounds(UVBounds uv_bounds);

	MeshId<TextureVertex2d> GetMeshId() const { return m_mesh_id; }
	Texture2dPipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }
	Bounds GetBounds() const { return Bounds{ m_center - m_size * 0.5f, m_center + m_size * 0.5f }; }

private:
	std::vector<TextureVertex2d> create_vertices() const;
	void update_mesh() const;

private:
	AssetManager * m_asset_manager = nullptr;
	MeshId<TextureVertex2d> m_mesh_id;
	Texture2dPipeline::ObjectData m_pipeline_data;
	glm::vec2 m_center{ 0.0f };
	glm::vec2 m_size{ 0.0f };
	UVBounds m_uv_bounds;
};

void UIImage::Init(
	AssetManager & asset_manager,
	AssetId texture_id,
	glm::vec2 center,
	glm::vec2 size)
{
	Init(asset_manager, texture_id, center, size, UVBounds{});
}

void UIImage::Init(
	AssetManager & asset_manager,
	AssetId texture_id,
	glm::vec2 center,
	glm::vec2 size,
	UVBounds uv_bounds /*= {}*/)
{
	m_asset_manager = &asset_manager;
	m_center = center;
	m_size = size;
	m_uv_bounds.min_u = uv_bounds.min_u;
	m_uv_bounds.max_u = uv_bounds.max_u;
	m_uv_bounds.min_v = uv_bounds.min_v;
	m_uv_bounds.max_v = uv_bounds.max_v;

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3,
	};

	m_mesh_id = asset_manager.AddMesh(create_vertices(), indices);
	m_pipeline_data = Texture2dPipeline::ObjectData{
		.tex_id = texture_id,
	};
}

void UIImage::SetUVBounds(UVBounds uv_bounds)
{
	m_uv_bounds.min_u = uv_bounds.min_u;
	m_uv_bounds.max_u = uv_bounds.max_u;
	m_uv_bounds.min_v = uv_bounds.min_v;
	m_uv_bounds.max_v = uv_bounds.max_v;
	update_mesh();
}

std::vector<TextureVertex2d> UIImage::create_vertices() const
{
	const glm::vec2 half_size = m_size * 0.5f;
	const glm::vec2 top_left = m_center - half_size;
	const glm::vec2 bottom_right = m_center + half_size;

	return std::vector<TextureVertex2d>{
		{ { top_left.x, top_left.y }, { m_uv_bounds.min_u, m_uv_bounds.min_v } },
		{ { bottom_right.x, top_left.y }, { m_uv_bounds.max_u, m_uv_bounds.min_v } },
		{ { top_left.x, bottom_right.y }, { m_uv_bounds.min_u, m_uv_bounds.max_v } },
		{ { bottom_right.x, bottom_right.y }, { m_uv_bounds.max_u, m_uv_bounds.max_v } },
	};
}

void UIImage::update_mesh() const
{
	if (!m_asset_manager || !m_mesh_id.IsValid())
		return;

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3,
	};
	m_asset_manager->UpdateMesh(m_mesh_id, create_vertices(), indices);
}
