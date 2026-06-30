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
	UIImage() = default;

	void Init(
		AssetManager & asset_manager,
		AssetId texture_id,
		glm::vec2 center,
		glm::vec2 size);

	MeshId<TextureVertex2d> GetMeshId() const { return m_mesh_id; }
	Texture2dPipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }

private:
	MeshId<TextureVertex2d> m_mesh_id;
	Texture2dPipeline::ObjectData m_pipeline_data;
};

void UIImage::Init(
	AssetManager & asset_manager,
	AssetId texture_id,
	glm::vec2 center,
	glm::vec2 size)
{
	const glm::vec2 half_size = size * 0.5f;
	const glm::vec2 top_left = center - half_size;
	const glm::vec2 bottom_right = center + half_size;

	std::vector<TextureVertex2d> vertices{
		{ { top_left.x, top_left.y }, { 0.0f, 0.0f } },
		{ { bottom_right.x, top_left.y }, { 1.0f, 0.0f } },
		{ { top_left.x, bottom_right.y }, { 0.0f, 1.0f } },
		{ { bottom_right.x, bottom_right.y }, { 1.0f, 1.0f } },
	};

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3,
	};

	m_mesh_id = asset_manager.AddMesh(vertices, indices);
	m_pipeline_data = Texture2dPipeline::ObjectData{
		.tex_id = texture_id,
	};
}
