// EditorGrid.ixx

module;

#include <vector>

export module EditorGrid;

import Dreamhearth;
using namespace Dreamhearth;

import AssetManager;
import LinePipeline;
import Vertex;

export class EditorGrid
{
public:
	void Init(AssetManager & asset_manager);

	MeshId<Vertex2d> GetMeshId() const { return mesh_id; }

private:
	MeshId<Vertex2d> create_grid_mesh(AssetManager & asset_manager);

private:
	MeshId<Vertex2d> mesh_id;
};

void EditorGrid::Init(AssetManager & asset_manager)
{
	mesh_id = create_grid_mesh(asset_manager);
}

MeshId<Vertex2d> EditorGrid::create_grid_mesh(AssetManager & asset_manager)
{
	std::vector<Vertex2d> verts{
		{ { -1.0,  1.0 } },
		{ {  1.0,  1.0 } },
		{ { -1.0, -1.0 } },
		{ {  1.0, -1.0 } } };

	std::vector<Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	std::vector<LineInstance> line_instances(20);
	for (int y = 0; y < 10; y++)
	{
		line_instances[y].p0 = { -4.0f, static_cast<float>(y) - 4.0f, 0.0f };
		line_instances[y].p1 = { 5.0f, static_cast<float>(y) - 4.0f, 0.0f };
		line_instances[y].thickness = 4.0f;
		line_instances[y].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	}
	for (int x = 0; x < 10; x++)
	{
		line_instances[x + 10].p0 = { static_cast<float>(x) - 4.0f, -4.0f, 0.0f };
		line_instances[x + 10].p1 = { static_cast<float>(x) - 4.0f, 5.0f, 0.0f };
		line_instances[x + 10].thickness = 4.0f;
		line_instances[x + 10].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	return asset_manager.AddMesh(verts, indices, line_instances);
}
