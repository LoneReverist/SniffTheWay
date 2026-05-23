// UIShadow.ixx

module;

#include <vector>

export module UIShadow;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import AssetPool;
import Vertex;

export class UIShadow
{
public:
	void Init(AssetManager & asset_manager, float left, float right, float top, float bottom);

	void SetROId(AssetId ro_id) { m_ro_id = ro_id; }

	MeshId<ColorVertex2d> GetMeshId() const { return m_mesh_id; }
	AssetId GetROId() const { return m_ro_id; }

private:
	MeshId<ColorVertex2d> m_mesh_id;
	AssetId m_ro_id;
};

void UIShadow::Init(AssetManager & asset_manager, float left, float right, float top, float bottom)
{
	std::vector<ColorVertex2d> verts{
		{ { left, top }, { 0.0f, 0.0f, 0.0f, 0.6f } },
		{ { right, top }, { 0.0f, 0.0f, 0.0f, 0.6f } },
		{ { left, bottom }, { 0.0f, 0.0f, 0.0f, 1.0f } },
		{ { right, bottom }, { 0.0f, 0.0f, 0.0f, 1.0f } } };

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	m_mesh_id = asset_manager.AddMesh(verts, indices);
}
