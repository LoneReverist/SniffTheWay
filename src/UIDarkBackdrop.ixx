// UIDarkBackdrop.ixx

module;

#include <vector>

export module UIDarkBackdrop;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Vertex;

namespace dh = Dreamhearth;

export class UIDarkBackdrop
{
public:
	void Init(AssetManager & asset_manager, float left, float right, float top, float bottom, float alpha_top, float alpha_bottom);

	void SetROId(AssetId ro_id) { m_ro_id = ro_id; }

	MeshId<ColorVertex2d> GetMeshId() const { return m_mesh_id; }
	AssetId GetROId() const { return m_ro_id; }

private:
	MeshId<ColorVertex2d> m_mesh_id;
	AssetId m_ro_id;
};

void UIDarkBackdrop::Init(AssetManager & asset_manager, float left, float right, float top, float bottom, float alpha_top, float alpha_bottom)
{
	std::vector<ColorVertex2d> verts{
		{ { left, top }, { 0.0f, 0.0f, 0.0f, alpha_top } },
		{ { right, top }, { 0.0f, 0.0f, 0.0f, alpha_top } },
		{ { left, bottom }, { 0.0f, 0.0f, 0.0f, alpha_bottom } },
		{ { right, bottom }, { 0.0f, 0.0f, 0.0f, alpha_bottom } } };

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	m_mesh_id = asset_manager.AddMesh(verts, indices);
}
