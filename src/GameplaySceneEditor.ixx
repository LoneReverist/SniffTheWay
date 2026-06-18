// GameplaySceneEditor.ixx

module;

#include <cstddef>
#include <vector>

#include <glm/glm.hpp>

export module GameplaySceneEditor;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Camera;
import EditorGrid;
import GameplaySceneData;
import Input;
import LinePipeline;
import Polygon2d;
import RenderObject;
import SceneRenderer;
import SniffTheWayConstants;
import Vertex;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class GameplaySceneEditor
{
public:
	void Init(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		Camera3d const & camera,
		GameplaySceneData const & scene_data);

	void Update(Input const & input, SceneRenderer & renderer, SceneState scene_state);
	void OnSceneStateChanged(SceneState new_state, SceneRenderer & renderer);
	void Reload(AssetManager & asset_manager, SceneRenderer & renderer);

private:
	MeshId<Vertex2d> create_bounds_mesh(AssetManager & asset_manager, Polygon2d const & bounds) const;
	std::vector<LineInstance> create_bounds_lines(Polygon2d const & bounds) const;

private:
	GameplaySceneData const * m_scene_data = nullptr;
	EditorGrid m_grid;
	MeshId<Vertex2d> m_bounds_mesh_id;
	AssetId m_bounds_ro_id;
};

void GameplaySceneEditor::Init(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	Camera3d const & camera,
	GameplaySceneData const & scene_data)
{
	m_scene_data = &scene_data;
	
	auto const line_pipeline_id = asset_manager.AddPipeline<LinePipeline>(camera);

	m_grid.Init(asset_manager);
	m_grid.SetROId(renderer.CreateRenderObject("editor grid", RenderLayer::Scene3d, m_grid.GetMeshId(), line_pipeline_id));

	m_bounds_mesh_id = create_bounds_mesh(asset_manager, m_scene_data->bounds);
	m_bounds_ro_id = renderer.CreateRenderObject("scene bounds", RenderLayer::Scene3d, m_bounds_mesh_id, line_pipeline_id);
	renderer.Show(m_bounds_ro_id, false);
}

void GameplaySceneEditor::Update(Input const & input, SceneRenderer & renderer, SceneState scene_state)
{
	m_grid.Update(input, renderer, scene_state);

	if (scene_state == SceneState::Gameplay && input.KeyJustPressed('B'))
		renderer.Show(m_bounds_ro_id, !renderer.IsShown(m_bounds_ro_id));
}

void GameplaySceneEditor::OnSceneStateChanged(SceneState new_state, SceneRenderer & renderer)
{
	m_grid.OnSceneStateChanged(new_state, renderer);
	if (new_state != SceneState::Gameplay)
		renderer.Show(m_bounds_ro_id, false);
}

void GameplaySceneEditor::Reload(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data)
		return;

	MeshId<Vertex2d> old_bounds_mesh_id = m_bounds_mesh_id;
	m_bounds_mesh_id = create_bounds_mesh(asset_manager, m_scene_data->bounds);

	RenderObject * bounds_ro = renderer.GetRenderObject(m_bounds_ro_id);
	if (bounds_ro)
		bounds_ro->SetMeshId(m_bounds_mesh_id);

	if (old_bounds_mesh_id.IsValid())
		asset_manager.RemoveMesh(old_bounds_mesh_id);
}

MeshId<Vertex2d> GameplaySceneEditor::create_bounds_mesh(AssetManager & asset_manager, Polygon2d const & bounds) const
{
	std::vector<Vertex2d> verts{
		{ { -1.0,  1.0 } },
		{ {  1.0,  1.0 } },
		{ { -1.0, -1.0 } },
		{ {  1.0, -1.0 } } };

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	return asset_manager.AddMesh(verts, indices, create_bounds_lines(bounds));
}

std::vector<LineInstance> GameplaySceneEditor::create_bounds_lines(Polygon2d const & bounds) const
{
	std::vector<glm::vec2> const & vertices = bounds.GetVertices();
	std::vector<LineInstance> lines;

	if (vertices.empty())
		return lines;

	constexpr float z = 0.04f;
	constexpr float bounds_thickness = 10.0f;
	constexpr float point_thickness = 12.0f;
	constexpr float point_radius = 0.05f;
	constexpr glm::vec4 bounds_color{ 1.0f, 0.82f, 0.18f, 0.9f };
	constexpr glm::vec4 point_color{ 1.0f, 0.25f, 0.15f, 1.0f };

	if (bounds.IsValid())
	{
		lines.reserve(vertices.size() * 3);
		for (std::size_t i = 0; i < vertices.size(); ++i)
		{
			glm::vec2 const & a = vertices[i];
			glm::vec2 const & b = vertices[(i + 1) % vertices.size()];
			lines.push_back(LineInstance{
				.p0 = { a.x, a.y, z },
				.p1 = { b.x, b.y, z },
				.thickness = bounds_thickness,
				.color = bounds_color
			});
		}
	}
	else
	{
		lines.reserve(vertices.size() * 2);
	}

	for (glm::vec2 const & vertex : vertices)
	{
		lines.push_back(LineInstance{
			.p0 = { vertex.x - point_radius, vertex.y, z },
			.p1 = { vertex.x + point_radius, vertex.y, z },
			.thickness = point_thickness,
			.color = point_color
		});
		lines.push_back(LineInstance{
			.p0 = { vertex.x, vertex.y - point_radius, z },
			.p1 = { vertex.x, vertex.y + point_radius, z },
			.thickness = point_thickness,
			.color = point_color
		});
	}

	return lines;
}
