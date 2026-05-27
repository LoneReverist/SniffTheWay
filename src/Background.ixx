// Background.ixx

module;

#include <iostream>
#include <string>

export module Background;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import AssetPool;
import BackgroundTexPipeline;
import Vertex;

export class Background
{
public:
	void Init(AssetManager & asset_manager, AssetId tex_id);
	void OnViewportResized(int width, int height, AssetManager & asset_manager);
	
	void SetTextureId(AssetId tex_id) { m_pipeline_data.tex_id = tex_id; }

	MeshId<TextureVertex2d> GetMeshId() const { return m_mesh_id; }
	BackgroundTexPipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }

private:
	MeshId<TextureVertex2d> create_bg_mesh(AssetManager & asset_manager);

private:
	MeshId<TextureVertex2d> m_mesh_id;
	BackgroundTexPipeline::ObjectData m_pipeline_data;
};

void Background::Init(AssetManager & asset_manager, AssetId tex_id)
{
	m_pipeline_data = BackgroundTexPipeline::ObjectData{
		.tex_id = tex_id
	};
	m_mesh_id = create_bg_mesh(asset_manager);
}

void Background::OnViewportResized(int width, int height, AssetManager & asset_manager)
{
	if (width == 0 || height == 0)
		return;

	dh::Texture const * bg_tex = asset_manager.GetTexture(m_pipeline_data.tex_id);
	if (!bg_tex)
		return;

	float world_scale = (static_cast<float>(height) / bg_tex->GetHeight());

	float y_size = 2.0f;
	float y_pos = -1.0f;
	float x_size = bg_tex->GetWidth() * (2.0f / width) * world_scale;
	float x_pos = -x_size / 2.0f;

	std::vector<TextureVertex2d> verts{
		{ { x_pos,          y_pos + y_size }, { 0.0, 0.0 } },
		{ { x_pos + x_size, y_pos + y_size }, { 1.0, 0.0 } },
		{ { x_pos,          y_pos          }, { 0.0, 1.0 } },
		{ { x_pos + x_size, y_pos          }, { 1.0, 1.0 } } };

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	asset_manager.UpdateMesh(m_mesh_id, verts, indices);
}

MeshId<TextureVertex2d> Background::create_bg_mesh(AssetManager & asset_manager)
{
	std::vector<TextureVertex2d> verts{
		{ { -1.0,  1.0 }, { 0.0, 0.0 } },
		{ {  1.0,  1.0 }, { 1.0, 0.0 } },
		{ { -1.0, -1.0 }, { 0.0, 1.0 } },
		{ {  1.0, -1.0 }, { 1.0, 1.0 } } };

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	return asset_manager.AddMesh(verts, indices);
}
