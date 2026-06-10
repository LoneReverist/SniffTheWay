// Background.ixx

module;

#include <iostream>
#include <string>

export module Background;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Texture2dPipeline;
import SniffTheWayConstants;
import Vertex;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class Background
{
public:
	void Init(AssetManager & asset_manager, AssetId tex_id);
	
	void SetTextureId(AssetId tex_id) { m_pipeline_data.tex_id = tex_id; }

	MeshId<TextureVertex2d> GetMeshId() const { return m_mesh_id; }
	Texture2dPipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }

private:
	MeshId<TextureVertex2d> create_bg_mesh(AssetManager & asset_manager);

private:
	MeshId<TextureVertex2d> m_mesh_id;
	Texture2dPipeline::ObjectData m_pipeline_data;
};

void Background::Init(AssetManager & asset_manager, AssetId tex_id)
{
	m_pipeline_data = Texture2dPipeline::ObjectData{
		.tex_id = tex_id
	};
	m_mesh_id = create_bg_mesh(asset_manager);
}

MeshId<TextureVertex2d> Background::create_bg_mesh(AssetManager & asset_manager)
{
	std::vector<TextureVertex2d> verts{
		{ { 0.0f, 0.0f }, { 0.0, 0.0 } },          // top-left
		{ { UIWidth, 0.0f }, { 1.0, 0.0 } },       // top-right
		{ { 0.0f, UIHeight}, { 0.0, 1.0 } },       // bottom-left
		{ { UIWidth, UIHeight }, { 1.0, 1.0 } } }; // bottom-right

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	return asset_manager.AddMesh(verts, indices);
}
