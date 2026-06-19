// GameplaySceneEditor.ixx

module;

#include <cstddef>
#include <filesystem>
#include <optional>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

export module GameplaySceneEditor;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Camera;
import EditorGrid;
import FontAtlas;
import GameplaySceneData;
import GameplaySceneLoader;
import Input;
import LinePipeline;
import Polygon2d;
import RenderObject;
import SceneRenderer;
import SniffTheWayConstants;
import TextPipeline;
import UILabel;
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
		FontAtlas const & font_atlas,
		PipelineId<TextPipeline> text_pipeline_id,
		GameplaySceneData & scene_data,
		std::filesystem::path scene_filepath);

	void Update(
		Input const & input,
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		Camera3d const & camera,
		glm::ivec4 viewport,
		SceneState scene_state);
	void OnSceneStateChanged(SceneState new_state, AssetManager & asset_manager, SceneRenderer & renderer);
	void Reload(AssetManager & asset_manager, SceneRenderer & renderer);

private:
	MeshId<Vertex2d> create_line_mesh(AssetManager & asset_manager, std::vector<LineInstance> const & lines) const;
	void rebuild_bounds_overlay(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		std::vector<glm::vec2> const & vertices,
		bool close_edges);
	void show_bounds(SceneRenderer & renderer, bool show);
	void show_bounds_editing_label(SceneRenderer & renderer, bool show);
	void begin_bounds_editing(AssetManager & asset_manager, SceneRenderer & renderer);
	void cancel_bounds_editing(AssetManager & asset_manager, SceneRenderer & renderer);
	bool apply_bounds_draft(AssetManager & asset_manager, SceneRenderer & renderer);
	void add_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 vertex);
	void remove_last_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer);
	bool save_scene_data() const;
	std::vector<LineInstance> create_edge_lines(std::vector<glm::vec2> const & vertices, bool close_edges) const;
	std::vector<LineInstance> create_point_lines(std::vector<glm::vec2> const & vertices) const;

private:
	GameplaySceneData * m_scene_data = nullptr;
	std::filesystem::path m_scene_filepath;
	EditorGrid m_grid;
	UILabel m_bounds_editing_label;
	bool m_is_editing_bounds = false;
	std::vector<glm::vec2> m_draft_vertices;
	MeshId<Vertex2d> m_bounds_edges_mesh_id;
	MeshId<Vertex2d> m_bounds_points_mesh_id;
	AssetId m_bounds_edges_ro_id;
	AssetId m_bounds_points_ro_id;
};

void GameplaySceneEditor::Init(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	Camera3d const & camera,
	FontAtlas const & font_atlas,
	PipelineId<TextPipeline> text_pipeline_id,
	GameplaySceneData & scene_data,
	std::filesystem::path scene_filepath)
{
	m_scene_data = &scene_data;
	m_scene_filepath = std::move(scene_filepath);
	
	auto const line_pipeline_id = asset_manager.AddPipeline<LinePipeline>(camera);

	m_grid.Init(asset_manager);
	m_grid.SetROId(renderer.CreateRenderObject("editor grid", RenderLayer::Scene3d, m_grid.GetMeshId(), line_pipeline_id));

	m_bounds_edges_mesh_id = create_line_mesh(asset_manager, create_edge_lines(m_scene_data->bounds.GetVertices(), m_scene_data->bounds.IsValid()));
	m_bounds_edges_ro_id = renderer.CreateRenderObject("scene bounds edges", RenderLayer::Scene3d, m_bounds_edges_mesh_id, line_pipeline_id);
	renderer.Show(m_bounds_edges_ro_id, false);

	m_bounds_points_mesh_id = create_line_mesh(asset_manager, create_point_lines(m_scene_data->bounds.GetVertices()));
	m_bounds_points_ro_id = renderer.CreateRenderObject("scene bounds vertices", RenderLayer::Scene3d, m_bounds_points_mesh_id, line_pipeline_id);
	renderer.Show(m_bounds_points_ro_id, false);

	m_bounds_editing_label.Init(asset_manager, "Editing bounds", font_atlas,
		LabelFontSize, glm::vec2{ 32.0f, 64.0f }, UILabel::Align::Left, StoryTextColor);
	m_bounds_editing_label.SetROId(renderer.CreateRenderObject("editing bounds label",
		RenderLayer::UIForeground, m_bounds_editing_label.GetMeshId(), text_pipeline_id, m_bounds_editing_label.GetPipelineData()));
	renderer.Show(m_bounds_editing_label.GetROId(), false);
}

void GameplaySceneEditor::Update(
	Input const & input,
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	Camera3d const & camera,
	glm::ivec4 viewport,
	SceneState scene_state)
{
	if (scene_state != SceneState::Editing)
		return;

	m_grid.Update(input, renderer, scene_state);

	const bool ctrl_is_down = input.KeyIsDown(Input::Key::LeftControl) || input.KeyIsDown(Input::Key::RightControl);
	if (ctrl_is_down && input.KeyJustPressed('S'))
	{
		if (m_is_editing_bounds && m_draft_vertices.size() >= 3)
			apply_bounds_draft(asset_manager, renderer);
		save_scene_data();
		return;
	}

	if (input.KeyJustPressed('B'))
	{
		if (m_is_editing_bounds)
			cancel_bounds_editing(asset_manager, renderer);
		else
			begin_bounds_editing(asset_manager, renderer);
		return;
	}

	if (m_is_editing_bounds)
	{
		if (input.KeyJustPressed(Input::Key::Backspace) || input.MouseButtonJustPressed(Input::MouseButton::Right))
		{
			remove_last_draft_vertex(asset_manager, renderer);
			return;
		}

		if (input.MouseButtonJustPressed(Input::MouseButton::Left))
		{
			if (std::optional<glm::vec2> ground_pos = camera.ScreenPointToGround(input.GetMousePos(), viewport))
				add_draft_vertex(asset_manager, renderer, *ground_pos);
			return;
		}

		if (input.KeyJustPressed(Input::Key::Enter))
		{
			apply_bounds_draft(asset_manager, renderer);
			return;
		}
	}
}

void GameplaySceneEditor::OnSceneStateChanged(SceneState new_state, AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (new_state == SceneState::Gameplay && m_is_editing_bounds)
		cancel_bounds_editing(asset_manager, renderer);

	m_grid.OnSceneStateChanged(new_state, renderer);
	show_bounds(renderer, new_state == SceneState::Editing);
	show_bounds_editing_label(renderer, new_state == SceneState::Editing && m_is_editing_bounds);
}

void GameplaySceneEditor::Reload(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data)
		return;

	m_is_editing_bounds = false;
	m_draft_vertices.clear();
	rebuild_bounds_overlay(asset_manager, renderer, m_scene_data->bounds.GetVertices(), m_scene_data->bounds.IsValid());
}

void GameplaySceneEditor::rebuild_bounds_overlay(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	std::vector<glm::vec2> const & vertices,
	bool close_edges)
{
	MeshId<Vertex2d> old_bounds_edges_mesh_id = m_bounds_edges_mesh_id;
	MeshId<Vertex2d> old_bounds_points_mesh_id = m_bounds_points_mesh_id;
	m_bounds_edges_mesh_id = create_line_mesh(asset_manager, create_edge_lines(vertices, close_edges));
	m_bounds_points_mesh_id = create_line_mesh(asset_manager, create_point_lines(vertices));

	RenderObject * bounds_edges_ro = renderer.GetRenderObject(m_bounds_edges_ro_id);
	if (bounds_edges_ro)
		bounds_edges_ro->SetMeshId(m_bounds_edges_mesh_id);

	RenderObject * bounds_points_ro = renderer.GetRenderObject(m_bounds_points_ro_id);
	if (bounds_points_ro)
		bounds_points_ro->SetMeshId(m_bounds_points_mesh_id);

	if (old_bounds_edges_mesh_id.IsValid())
		asset_manager.RemoveMesh(old_bounds_edges_mesh_id);
	if (old_bounds_points_mesh_id.IsValid())
		asset_manager.RemoveMesh(old_bounds_points_mesh_id);
}

void GameplaySceneEditor::show_bounds(SceneRenderer & renderer, bool show)
{
	renderer.Show(m_bounds_edges_ro_id, show);
	renderer.Show(m_bounds_points_ro_id, show);
}

void GameplaySceneEditor::show_bounds_editing_label(SceneRenderer & renderer, bool show)
{
	renderer.Show(m_bounds_editing_label.GetROId(), show);
}

void GameplaySceneEditor::begin_bounds_editing(AssetManager & asset_manager, SceneRenderer & renderer)
{
	m_is_editing_bounds = true;
	m_draft_vertices = m_scene_data ? m_scene_data->bounds.GetVertices() : std::vector<glm::vec2>{};
	rebuild_bounds_overlay(asset_manager, renderer, m_draft_vertices, m_draft_vertices.size() >= 3);
	show_bounds(renderer, true);
	show_bounds_editing_label(renderer, true);
}

void GameplaySceneEditor::cancel_bounds_editing(AssetManager & asset_manager, SceneRenderer & renderer)
{
	m_is_editing_bounds = false;
	m_draft_vertices.clear();
	if (m_scene_data)
		rebuild_bounds_overlay(asset_manager, renderer, m_scene_data->bounds.GetVertices(), m_scene_data->bounds.IsValid());
	show_bounds(renderer, true);
	show_bounds_editing_label(renderer, false);
}

bool GameplaySceneEditor::apply_bounds_draft(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data || m_draft_vertices.size() < 3)
		return false;

	m_scene_data->bounds.SetVertices(m_draft_vertices);
	m_is_editing_bounds = false;
	m_draft_vertices.clear();
	rebuild_bounds_overlay(asset_manager, renderer, m_scene_data->bounds.GetVertices(), m_scene_data->bounds.IsValid());
	show_bounds(renderer, true);
	show_bounds_editing_label(renderer, false);
	return true;
}

void GameplaySceneEditor::add_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 vertex)
{
	m_draft_vertices.push_back(vertex);
	rebuild_bounds_overlay(asset_manager, renderer, m_draft_vertices, m_draft_vertices.size() >= 3);
}

void GameplaySceneEditor::remove_last_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (m_draft_vertices.empty())
		return;

	m_draft_vertices.pop_back();
	rebuild_bounds_overlay(asset_manager, renderer, m_draft_vertices, m_draft_vertices.size() >= 3);
}

bool GameplaySceneEditor::save_scene_data() const
{
	return m_scene_data && GameplaySceneLoader::SaveSceneData(m_scene_filepath, *m_scene_data);
}

MeshId<Vertex2d> GameplaySceneEditor::create_line_mesh(AssetManager & asset_manager, std::vector<LineInstance> const & lines) const
{
	std::vector<LineInstance> mesh_lines = lines;
	if (mesh_lines.empty())
	{
		mesh_lines.push_back(LineInstance{
			.p0 = { 0.0f, 0.0f, 0.0f },
			.p1 = { 0.0f, 0.0f, 0.0f },
			.thickness = 0.0f,
			.color = { 0.0f, 0.0f, 0.0f, 0.0f }
		});
	}

	std::vector<Vertex2d> verts{
		{ { -1.0,  1.0 } },
		{ {  1.0,  1.0 } },
		{ { -1.0, -1.0 } },
		{ {  1.0, -1.0 } } };

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	return asset_manager.AddMesh(verts, indices, mesh_lines);
}

std::vector<LineInstance> GameplaySceneEditor::create_edge_lines(std::vector<glm::vec2> const & vertices, bool close_edges) const
{
	std::vector<LineInstance> lines;

	const std::size_t edge_count = close_edges
		? vertices.size()
		: vertices.size() > 1 ? vertices.size() - 1 : 0;
	if (edge_count == 0)
		return lines;

	constexpr float bounds_thickness = 10.0f;
	constexpr glm::vec4 bounds_color{ 1.0f, 0.82f, 0.18f, 0.9f };

	lines.reserve(edge_count);
	for (std::size_t i = 0; i < edge_count; ++i)
	{
		glm::vec2 const & a = vertices[i];
		glm::vec2 const & b = vertices[(i + 1) % vertices.size()];
		lines.push_back(LineInstance{
			.p0 = { a.x, a.y, 0.0f },
			.p1 = { b.x, b.y, 0.0f },
			.thickness = bounds_thickness,
			.color = bounds_color
		});
	}

	return lines;
}

std::vector<LineInstance> GameplaySceneEditor::create_point_lines(std::vector<glm::vec2> const & vertices) const
{
	std::vector<LineInstance> lines;
	lines.reserve(vertices.size() * 2);

	constexpr float point_thickness = 12.0f;
	constexpr float point_radius = 0.05f;
	constexpr glm::vec4 point_color{ 1.0f, 0.25f, 0.15f, 1.0f };

	for (glm::vec2 const & vertex : vertices)
	{
		lines.push_back(LineInstance{
			.p0 = { vertex.x - point_radius, vertex.y, 0.0f },
			.p1 = { vertex.x + point_radius, vertex.y, 0.0f },
			.thickness = point_thickness,
			.color = point_color
		});
		lines.push_back(LineInstance{
			.p0 = { vertex.x, vertex.y - point_radius, 0.0f },
			.p1 = { vertex.x, vertex.y + point_radius, 0.0f },
			.thickness = point_thickness,
			.color = point_color
		});
	}

	return lines;
}
