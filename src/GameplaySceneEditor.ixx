// GameplaySceneEditor.ixx

module;

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
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
	bool HasActiveEditMode() const { return m_is_editing_polygon || m_is_editing_spawn; }
	void OnSceneStateChanged(SceneState new_state, AssetManager & asset_manager, SceneRenderer & renderer);
	void Reload(AssetManager & asset_manager, SceneRenderer & renderer);

private:
	enum class PolygonEditTargetKind
	{
		SceneBounds,
		SceneLink,
		ScentTrail,
	};

	struct PolygonEditTarget
	{
		PolygonEditTargetKind kind = PolygonEditTargetKind::SceneBounds;
		std::size_t link_index = 0;
	};

	struct PolygonOverlay
	{
		MeshId<Vertex2d> edges_mesh_id;
		MeshId<Vertex2d> points_mesh_id;
		AssetId edges_ro_id;
		AssetId points_ro_id;
	};

	enum class SpawnCharacter
	{
		Dog,
		Baby,
	};

	struct SpawnEditTarget
	{
		SpawnCharacter character = SpawnCharacter::Dog;
		std::size_t link_index = 0;
	};

private:
	MeshId<Vertex2d> create_line_mesh(AssetManager & asset_manager, std::vector<LineInstance> const & lines) const;
	PolygonOverlay create_polygon_overlay(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		std::string const & name,
		std::vector<glm::vec2> const & vertices,
		bool close_edges,
		glm::vec4 edge_color,
		glm::vec4 point_color,
		float edge_thickness,
		float point_thickness,
		bool show);
	void rebuild_polygon_overlay(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		PolygonOverlay & overlay,
		std::vector<glm::vec2> const & vertices,
		bool close_edges,
		glm::vec4 edge_color,
		glm::vec4 point_color,
		float edge_thickness,
		float point_thickness);
	void show_polygon_overlay(SceneRenderer & renderer, PolygonOverlay const & overlay, bool show);
	void rebuild_bounds_overlay(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		std::vector<glm::vec2> const & vertices,
		bool close_edges);
	void rebuild_scent_trail_overlay(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		std::vector<glm::vec2> const & points);
	void rebuild_scene_link_overlays(AssetManager & asset_manager, SceneRenderer & renderer);
	void rebuild_scene_link_overlay(AssetManager & asset_manager, SceneRenderer & renderer, std::size_t index);
	void rebuild_spawn_markers(AssetManager & asset_manager, SceneRenderer & renderer);
	void show_bounds(SceneRenderer & renderer, bool show);
	void show_scent_trail(SceneRenderer & renderer, bool show);
	void show_scene_link_triggers(SceneRenderer & renderer, bool show);
	void show_spawn_markers(SceneRenderer & renderer, bool show);
	void rebuild_selected_vertex_marker(AssetManager & asset_manager, SceneRenderer & renderer);
	void show_selected_vertex_marker(SceneRenderer & renderer, bool show);
	void show_polygon_editing_label(SceneRenderer & renderer, bool show);
	void begin_polygon_editing(AssetManager & asset_manager, SceneRenderer & renderer, PolygonEditTarget target);
	void cancel_polygon_editing(AssetManager & asset_manager, SceneRenderer & renderer);
	bool apply_polygon_draft(AssetManager & asset_manager, SceneRenderer & renderer);
	void add_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 vertex);
	void remove_last_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer);
	void select_next_polygon_vertex(AssetManager & asset_manager, SceneRenderer & renderer, int direction);
	void append_polygon_vertex(AssetManager & asset_manager, SceneRenderer & renderer);
	void delete_selected_polygon_vertex(AssetManager & asset_manager, SceneRenderer & renderer);
	void nudge_selected_polygon_vertex(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 delta);
	void move_selected_polygon_vertex_to(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 position);
	std::optional<std::size_t> find_nearest_draft_vertex(glm::vec2 point, float max_distance) const;
	void update_selected_vertex_index_after_size_change();
	void rebuild_edit_target_overlay(AssetManager & asset_manager, SceneRenderer & renderer);
	void begin_spawn_editing(AssetManager & asset_manager, SceneRenderer & renderer, SpawnEditTarget target);
	void cancel_spawn_editing(AssetManager & asset_manager, SceneRenderer & renderer);
	void set_spawn_position(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 pos);
	void select_next_scene_link(AssetManager & asset_manager, SceneRenderer & renderer);
	void create_scene_link(AssetManager & asset_manager, SceneRenderer & renderer);
	void delete_selected_scene_link(AssetManager & asset_manager, SceneRenderer & renderer);
	bool has_selected_scene_link() const;
	PolygonEditTarget selected_scene_link_target() const;
	SpawnEditTarget selected_scene_link_spawn_target(SpawnCharacter character) const;
	std::vector<glm::vec2> get_target_vertices(PolygonEditTarget target) const;
	void set_target_vertices(PolygonEditTarget target, std::vector<glm::vec2> vertices);
	glm::vec2 get_spawn_position(SpawnEditTarget target) const;
	void set_spawn_position(SpawnEditTarget target, glm::vec2 pos);
	void update_polygon_editing_label();
	std::string create_editor_label_text() const;
	bool save_scene_data() const;
	std::vector<LineInstance> create_edge_lines(
		std::vector<glm::vec2> const & vertices,
		bool close_edges,
		glm::vec4 color,
		float thickness) const;
	std::vector<LineInstance> create_point_lines(
		std::vector<glm::vec2> const & vertices,
		glm::vec4 color,
		float thickness) const;
	std::vector<LineInstance> create_spawn_marker_lines() const;
	std::vector<LineInstance> create_selected_vertex_marker_lines() const;
	void append_spawn_marker_lines(
		std::vector<LineInstance> & lines,
		glm::vec2 pos,
		glm::vec4 color,
		float size,
		float thickness) const;

public:
	bool ConsumeScentTrailChanged();

private:
	GameplaySceneData * m_scene_data = nullptr;
	std::filesystem::path m_scene_filepath;
	EditorGrid m_grid;
	UILabel m_polygon_editing_label;
	PipelineId<LinePipeline> m_line_pipeline_id;
	bool m_is_editing_polygon = false;
	std::optional<PolygonEditTarget> m_edit_target;
	bool m_is_editing_spawn = false;
	std::optional<SpawnEditTarget> m_spawn_edit_target;
	std::optional<std::size_t> m_selected_scene_link_index;
	std::optional<std::size_t> m_selected_vertex_index;
	std::optional<std::size_t> m_dragged_vertex_index;
	glm::vec2 m_dragged_vertex_offset{ 0.0f };
	std::vector<glm::vec2> m_draft_vertices;
	PolygonOverlay m_bounds_overlay;
	PolygonOverlay m_scent_trail_overlay;
	std::vector<PolygonOverlay> m_scene_link_overlays;
	MeshId<Vertex2d> m_selected_vertex_marker_mesh_id;
	AssetId m_selected_vertex_marker_ro_id;
	MeshId<Vertex2d> m_spawn_markers_mesh_id;
	AssetId m_spawn_markers_ro_id;
	bool m_scent_trail_changed = false;
};

namespace
{
	constexpr float BoundsEdgeThickness = 10.0f;
	constexpr float BoundsPointThickness = 12.0f;
	constexpr float LinkTriggerEdgeThickness = 8.0f;
	constexpr float LinkTriggerPointThickness = 10.0f;
	constexpr glm::vec4 BoundsEdgeColor{ 1.0f, 0.82f, 0.18f, 0.9f };
	constexpr glm::vec4 BoundsPointColor{ 1.0f, 0.25f, 0.15f, 1.0f };
	constexpr glm::vec4 LinkTriggerEdgeColor{ 0.1f, 0.75f, 1.0f, 0.75f };
	constexpr glm::vec4 LinkTriggerPointColor{ 0.2f, 0.9f, 1.0f, 0.95f };
	constexpr glm::vec4 SelectedLinkTriggerEdgeColor{ 0.2f, 1.0f, 0.45f, 0.95f };
	constexpr glm::vec4 SelectedLinkTriggerPointColor{ 0.1f, 1.0f, 0.25f, 1.0f };
	constexpr glm::vec4 ScentTrailEdgeColor{ 1.0f, 0.82f, 0.25f, 0.75f };
	constexpr glm::vec4 ScentTrailPointColor{ 1.0f, 0.95f, 0.45f, 1.0f };
	constexpr glm::vec4 EditingScentTrailEdgeColor{ 1.0f, 0.55f, 0.08f, 0.95f };
	constexpr glm::vec4 EditingScentTrailPointColor{ 1.0f, 0.92f, 0.2f, 1.0f };
	constexpr glm::vec4 SelectedVertexColor{ 0.2f, 1.0f, 0.45f, 1.0f };
	constexpr glm::vec4 DraftEdgeColor{ 1.0f, 0.55f, 0.08f, 0.95f };
	constexpr glm::vec4 DraftPointColor{ 1.0f, 0.15f, 0.1f, 1.0f };
	constexpr float ScentTrailEdgeThickness = 8.0f;
	constexpr float ScentTrailPointThickness = 12.0f;
	constexpr float SelectedVertexMarkerSize = 0.18f;
	constexpr float SelectedVertexMarkerThickness = 14.0f;
	constexpr float PolygonAppendDistance = 0.75f;
	constexpr float PolygonNudgeDistance = 0.04f;
	constexpr float PolygonFineNudgeDistance = 0.01f;
	constexpr float PolygonCoarseNudgeDistance = 0.12f;
	constexpr float PolygonDragHitRadius = 0.28f;
	constexpr glm::vec4 LinkDogArrivalColor{ 0.25f, 0.85f, 1.0f, 0.8f };
	constexpr glm::vec4 LinkBabyArrivalColor{ 0.85f, 0.45f, 1.0f, 0.8f };
	constexpr glm::vec4 SelectedSpawnColor{ 0.2f, 1.0f, 0.45f, 1.0f };
	constexpr glm::vec4 EditingSpawnColor{ 1.0f, 0.55f, 0.08f, 1.0f };
	constexpr float SpawnMarkerSize = 0.16f;
	constexpr float SpawnMarkerThickness = 10.0f;
	constexpr float SelectedSpawnMarkerSize = 0.22f;
	constexpr float SelectedSpawnMarkerThickness = 14.0f;
}

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
	
	m_line_pipeline_id = asset_manager.AddPipeline<LinePipeline>(camera);

	m_grid.Init(asset_manager);
	m_grid.SetROId(renderer.CreateRenderObject("editor grid", RenderLayer::Scene3d, m_grid.GetMeshId(), m_line_pipeline_id));

	m_bounds_overlay = create_polygon_overlay(
		asset_manager,
		renderer,
		"scene bounds",
		m_scene_data->bounds.GetVertices(),
		m_scene_data->bounds.IsValid(),
		BoundsEdgeColor,
		BoundsPointColor,
		BoundsEdgeThickness,
		BoundsPointThickness,
		false);

	m_scent_trail_overlay = create_polygon_overlay(
		asset_manager,
		renderer,
		"scent trail",
		m_scene_data->scent_trail.points,
		false,
		ScentTrailEdgeColor,
		ScentTrailPointColor,
		ScentTrailEdgeThickness,
		ScentTrailPointThickness,
		false);

	m_selected_vertex_marker_mesh_id = create_line_mesh(asset_manager, create_selected_vertex_marker_lines());
	m_selected_vertex_marker_ro_id = renderer.CreateRenderObject("selected polygon vertex",
		RenderLayer::Scene3d, m_selected_vertex_marker_mesh_id, m_line_pipeline_id);
	renderer.Show(m_selected_vertex_marker_ro_id, false);

	rebuild_scene_link_overlays(asset_manager, renderer);

	m_spawn_markers_mesh_id = create_line_mesh(asset_manager, create_spawn_marker_lines());
	m_spawn_markers_ro_id = renderer.CreateRenderObject("spawn markers", RenderLayer::Scene3d, m_spawn_markers_mesh_id, m_line_pipeline_id);
	renderer.Show(m_spawn_markers_ro_id, false);

	m_polygon_editing_label.Init(asset_manager, create_editor_label_text(), font_atlas,
		LabelFontSize, glm::vec2{ 32.0f, 64.0f }, UILabel::Align::Left, StoryTextColor);
	m_polygon_editing_label.SetROId(renderer.CreateRenderObject("editing polygon label",
		RenderLayer::UIForeground, m_polygon_editing_label.GetMeshId(), text_pipeline_id, m_polygon_editing_label.GetPipelineData()));
	renderer.Show(m_polygon_editing_label.GetROId(), false);
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
	const bool shift_is_down = input.KeyIsDown(Input::Key::LeftShift) || input.KeyIsDown(Input::Key::RightShift);
	if (ctrl_is_down && input.KeyJustPressed('S') && !m_is_editing_polygon && !m_is_editing_spawn)
	{
		save_scene_data();
		return;
	}

	if (m_is_editing_spawn)
	{
		if (input.KeyJustPressed(Input::Key::Esc))
		{
			cancel_spawn_editing(asset_manager, renderer);
			return;
		}

		if (input.MouseButtonJustPressed(Input::MouseButton::Left))
		{
			if (std::optional<glm::vec2> ground_pos = camera.ScreenPointToGround(input.GetMousePos(), viewport))
				set_spawn_position(asset_manager, renderer, *ground_pos);
			return;
		}

		return;
	}

	if (m_is_editing_polygon)
	{
		if (input.KeyJustPressed(Input::Key::Esc))
		{
			cancel_polygon_editing(asset_manager, renderer);
			return;
		}

		if (m_edit_target)
		{
			if (input.MouseButtonJustReleased(Input::MouseButton::Left))
			{
				m_dragged_vertex_index.reset();
				return;
			}

			if (input.MouseButtonJustPressed(Input::MouseButton::Left))
			{
				if (std::optional<glm::vec2> ground_pos = camera.ScreenPointToGround(input.GetMousePos(), viewport))
				{
					if (std::optional<std::size_t> vertex_index = find_nearest_draft_vertex(*ground_pos, PolygonDragHitRadius))
					{
						m_selected_vertex_index = *vertex_index;
						m_dragged_vertex_index = *vertex_index;
						m_dragged_vertex_offset = m_draft_vertices[*vertex_index] - *ground_pos;
						rebuild_selected_vertex_marker(asset_manager, renderer);
						show_selected_vertex_marker(renderer, true);
						update_polygon_editing_label();
					}
				}
				return;
			}

			if (m_dragged_vertex_index)
			{
				if (!input.MouseButtonIsDown(Input::MouseButton::Left))
				{
					m_dragged_vertex_index.reset();
					return;
				}

				if (*m_dragged_vertex_index < m_draft_vertices.size())
				{
					m_selected_vertex_index = *m_dragged_vertex_index;
					if (std::optional<glm::vec2> ground_pos = camera.ScreenPointToGround(input.GetMousePos(), viewport))
						move_selected_polygon_vertex_to(asset_manager, renderer, *ground_pos + m_dragged_vertex_offset);
				}
				else
				{
					m_dragged_vertex_index.reset();
				}
				return;
			}

			if (input.KeyJustPressed(']'))
			{
				select_next_polygon_vertex(asset_manager, renderer, 1);
				return;
			}

			if (input.KeyJustPressed('['))
			{
				select_next_polygon_vertex(asset_manager, renderer, -1);
				return;
			}

			if (input.KeyJustPressed('N'))
			{
				append_polygon_vertex(asset_manager, renderer);
				return;
			}

			if (input.KeyJustPressed(Input::Key::Delete))
			{
				delete_selected_polygon_vertex(asset_manager, renderer);
				return;
			}

			const float nudge_distance = ctrl_is_down
				? PolygonFineNudgeDistance
				: shift_is_down ? PolygonCoarseNudgeDistance : PolygonNudgeDistance;
			glm::vec2 nudge{ 0.0f };
			if (input.KeyIsDown('W') || input.KeyIsDown(Input::Key::Up))
				nudge.y += nudge_distance;
			if (input.KeyIsDown('S') || input.KeyIsDown(Input::Key::Down))
				nudge.y -= nudge_distance;
			if (input.KeyIsDown('A') || input.KeyIsDown(Input::Key::Left))
				nudge.x -= nudge_distance;
			if (input.KeyIsDown('D') || input.KeyIsDown(Input::Key::Right))
				nudge.x += nudge_distance;

			if (nudge != glm::vec2{ 0.0f })
			{
				nudge_selected_polygon_vertex(asset_manager, renderer, nudge);
				return;
			}
		}

		if (input.KeyJustPressed(Input::Key::Tab))
			return;

		if (input.KeyJustPressed(Input::Key::Enter))
		{
			apply_polygon_draft(asset_manager, renderer);
			return;
		}

		return;
	}

	if (input.KeyJustPressed('B'))
	{
		if (!m_is_editing_polygon || !m_edit_target || m_edit_target->kind != PolygonEditTargetKind::SceneBounds)
			begin_polygon_editing(asset_manager, renderer, PolygonEditTarget{ .kind = PolygonEditTargetKind::SceneBounds });
		return;
	}

	if (input.KeyJustPressed('T'))
	{
		if (!m_is_editing_polygon || !m_edit_target || m_edit_target->kind != PolygonEditTargetKind::ScentTrail)
			begin_polygon_editing(asset_manager, renderer, PolygonEditTarget{ .kind = PolygonEditTargetKind::ScentTrail });
		return;
	}

	if (input.KeyJustPressed(Input::Key::Tab))
	{
		select_next_scene_link(asset_manager, renderer);
		return;
	}

	if (shift_is_down && input.KeyJustPressed('L'))
	{
		create_scene_link(asset_manager, renderer);
		return;
	}

	if (input.KeyJustPressed('L'))
	{
		if ((!m_is_editing_polygon || !m_edit_target || m_edit_target->kind != PolygonEditTargetKind::SceneLink)
			&& has_selected_scene_link())
		{
			begin_polygon_editing(asset_manager, renderer, selected_scene_link_target());
		}
		return;
	}

	if (input.KeyJustPressed(Input::Key::Delete))
	{
		delete_selected_scene_link(asset_manager, renderer);
		return;
	}

	if (input.KeyJustPressed('1'))
	{
		if (m_is_editing_spawn
			&& m_spawn_edit_target
			&& m_spawn_edit_target->character == SpawnCharacter::Dog)
			cancel_spawn_editing(asset_manager, renderer);
		else if (has_selected_scene_link())
			begin_spawn_editing(asset_manager, renderer, selected_scene_link_spawn_target(SpawnCharacter::Dog));
		return;
	}

	if (input.KeyJustPressed('2'))
	{
		if (m_is_editing_spawn
			&& m_spawn_edit_target
			&& m_spawn_edit_target->character == SpawnCharacter::Baby)
			cancel_spawn_editing(asset_manager, renderer);
		else if (has_selected_scene_link())
			begin_spawn_editing(asset_manager, renderer, selected_scene_link_spawn_target(SpawnCharacter::Baby));
		return;
	}
}

void GameplaySceneEditor::OnSceneStateChanged(SceneState new_state, AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (new_state == SceneState::Gameplay && m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);
	if (new_state == SceneState::Gameplay && m_is_editing_spawn)
		cancel_spawn_editing(asset_manager, renderer);

	m_grid.OnSceneStateChanged(new_state, renderer);
	show_bounds(renderer, new_state == SceneState::Editing);
	show_scent_trail(renderer, new_state == SceneState::Editing);
	show_selected_vertex_marker(renderer, new_state == SceneState::Editing);
	show_scene_link_triggers(renderer, new_state == SceneState::Editing);
	show_spawn_markers(renderer, new_state == SceneState::Editing);
	update_polygon_editing_label();
	show_polygon_editing_label(renderer, new_state == SceneState::Editing);
}

void GameplaySceneEditor::Reload(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data)
		return;

	m_is_editing_polygon = false;
	m_edit_target.reset();
	m_is_editing_spawn = false;
	m_spawn_edit_target.reset();
	m_selected_vertex_index.reset();
	m_dragged_vertex_index.reset();
	m_draft_vertices.clear();
	if (m_selected_scene_link_index && *m_selected_scene_link_index >= m_scene_data->scene_links.size())
		m_selected_scene_link_index.reset();

	rebuild_bounds_overlay(asset_manager, renderer, m_scene_data->bounds.GetVertices(), m_scene_data->bounds.IsValid());
	rebuild_scent_trail_overlay(asset_manager, renderer, m_scene_data->scent_trail.points);
	rebuild_selected_vertex_marker(asset_manager, renderer);
	rebuild_scene_link_overlays(asset_manager, renderer);
	rebuild_spawn_markers(asset_manager, renderer);
	update_polygon_editing_label();
}

GameplaySceneEditor::PolygonOverlay GameplaySceneEditor::create_polygon_overlay(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	std::string const & name,
	std::vector<glm::vec2> const & vertices,
	bool close_edges,
	glm::vec4 edge_color,
	glm::vec4 point_color,
	float edge_thickness,
	float point_thickness,
	bool show)
{
	PolygonOverlay overlay;
	overlay.edges_mesh_id = create_line_mesh(asset_manager, create_edge_lines(vertices, close_edges, edge_color, edge_thickness));
	overlay.edges_ro_id = renderer.CreateRenderObject(name + " edges", RenderLayer::Scene3d, overlay.edges_mesh_id, m_line_pipeline_id);
	renderer.Show(overlay.edges_ro_id, show);

	overlay.points_mesh_id = create_line_mesh(asset_manager, create_point_lines(vertices, point_color, point_thickness));
	overlay.points_ro_id = renderer.CreateRenderObject(name + " vertices", RenderLayer::Scene3d, overlay.points_mesh_id, m_line_pipeline_id);
	renderer.Show(overlay.points_ro_id, show);
	return overlay;
}

void GameplaySceneEditor::rebuild_polygon_overlay(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	PolygonOverlay & overlay,
	std::vector<glm::vec2> const & vertices,
	bool close_edges,
	glm::vec4 edge_color,
	glm::vec4 point_color,
	float edge_thickness,
	float point_thickness)
{
	MeshId<Vertex2d> old_edges_mesh_id = overlay.edges_mesh_id;
	MeshId<Vertex2d> old_points_mesh_id = overlay.points_mesh_id;
	overlay.edges_mesh_id = create_line_mesh(asset_manager, create_edge_lines(vertices, close_edges, edge_color, edge_thickness));
	overlay.points_mesh_id = create_line_mesh(asset_manager, create_point_lines(vertices, point_color, point_thickness));

	RenderObject * edges_ro = renderer.GetRenderObject(overlay.edges_ro_id);
	if (edges_ro)
		edges_ro->SetMeshId(overlay.edges_mesh_id);

	RenderObject * points_ro = renderer.GetRenderObject(overlay.points_ro_id);
	if (points_ro)
		points_ro->SetMeshId(overlay.points_mesh_id);

	if (old_edges_mesh_id.IsValid())
		asset_manager.RemoveMesh(old_edges_mesh_id);
	if (old_points_mesh_id.IsValid())
		asset_manager.RemoveMesh(old_points_mesh_id);
}

void GameplaySceneEditor::show_polygon_overlay(SceneRenderer & renderer, PolygonOverlay const & overlay, bool show)
{
	renderer.Show(overlay.edges_ro_id, show);
	renderer.Show(overlay.points_ro_id, show);
}

void GameplaySceneEditor::rebuild_bounds_overlay(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	std::vector<glm::vec2> const & vertices,
	bool close_edges)
{
	glm::vec4 edge_color = (m_is_editing_polygon && m_edit_target && m_edit_target->kind == PolygonEditTargetKind::SceneBounds)
		? DraftEdgeColor
		: BoundsEdgeColor;
	glm::vec4 point_color = (m_is_editing_polygon && m_edit_target && m_edit_target->kind == PolygonEditTargetKind::SceneBounds)
		? DraftPointColor
		: BoundsPointColor;

	rebuild_polygon_overlay(
		asset_manager,
		renderer,
		m_bounds_overlay,
		vertices,
		close_edges,
		edge_color,
		point_color,
		BoundsEdgeThickness,
		BoundsPointThickness);
}

void GameplaySceneEditor::rebuild_scent_trail_overlay(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	std::vector<glm::vec2> const & points)
{
	glm::vec4 edge_color = (m_is_editing_polygon && m_edit_target && m_edit_target->kind == PolygonEditTargetKind::ScentTrail)
		? EditingScentTrailEdgeColor
		: ScentTrailEdgeColor;
	glm::vec4 point_color = (m_is_editing_polygon && m_edit_target && m_edit_target->kind == PolygonEditTargetKind::ScentTrail)
		? EditingScentTrailPointColor
		: ScentTrailPointColor;

	rebuild_polygon_overlay(
		asset_manager,
		renderer,
		m_scent_trail_overlay,
		points,
		false,
		edge_color,
		point_color,
		ScentTrailEdgeThickness,
		ScentTrailPointThickness);
}

void GameplaySceneEditor::rebuild_scene_link_overlays(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data)
		return;

	for (std::size_t i = m_scene_data->scene_links.size(); i < m_scene_link_overlays.size(); ++i)
		show_polygon_overlay(renderer, m_scene_link_overlays[i], false);

	while (m_scene_link_overlays.size() < m_scene_data->scene_links.size())
	{
		std::size_t const index = m_scene_link_overlays.size();
		GameplaySceneLink const & scene_link = m_scene_data->scene_links[index];
		m_scene_link_overlays.push_back(create_polygon_overlay(
			asset_manager,
			renderer,
			"scene link " + std::to_string(index + 1),
			scene_link.trigger.GetVertices(),
			scene_link.trigger.IsValid(),
			LinkTriggerEdgeColor,
			LinkTriggerPointColor,
			LinkTriggerEdgeThickness,
			LinkTriggerPointThickness,
			false));
	}

	for (std::size_t i = 0; i < m_scene_data->scene_links.size(); ++i)
	{
		rebuild_scene_link_overlay(asset_manager, renderer, i);
		show_polygon_overlay(renderer, m_scene_link_overlays[i], true);
	}
}

void GameplaySceneEditor::rebuild_scene_link_overlay(AssetManager & asset_manager, SceneRenderer & renderer, std::size_t index)
{
	if (!m_scene_data || index >= m_scene_data->scene_links.size() || index >= m_scene_link_overlays.size())
		return;

	bool const is_selected = m_selected_scene_link_index && *m_selected_scene_link_index == index;
	bool const is_editing_this = m_is_editing_polygon
		&& m_edit_target
		&& m_edit_target->kind == PolygonEditTargetKind::SceneLink
		&& m_edit_target->link_index == index;
	glm::vec4 edge_color = is_editing_this ? DraftEdgeColor : is_selected ? SelectedLinkTriggerEdgeColor : LinkTriggerEdgeColor;
	glm::vec4 point_color = is_editing_this ? DraftPointColor : is_selected ? SelectedLinkTriggerPointColor : LinkTriggerPointColor;

	std::vector<glm::vec2> const & vertices = is_editing_this
		? m_draft_vertices
		: m_scene_data->scene_links[index].trigger.GetVertices();
	bool const close_edges = is_editing_this
		? m_draft_vertices.size() >= 3
		: m_scene_data->scene_links[index].trigger.IsValid();

	rebuild_polygon_overlay(
		asset_manager,
		renderer,
		m_scene_link_overlays[index],
		vertices,
		close_edges,
		edge_color,
		point_color,
		LinkTriggerEdgeThickness,
		LinkTriggerPointThickness);
}

void GameplaySceneEditor::rebuild_spawn_markers(AssetManager & asset_manager, SceneRenderer & renderer)
{
	MeshId<Vertex2d> old_spawn_markers_mesh_id = m_spawn_markers_mesh_id;
	m_spawn_markers_mesh_id = create_line_mesh(asset_manager, create_spawn_marker_lines());

	RenderObject * spawn_markers_ro = renderer.GetRenderObject(m_spawn_markers_ro_id);
	if (spawn_markers_ro)
		spawn_markers_ro->SetMeshId(m_spawn_markers_mesh_id);

	if (old_spawn_markers_mesh_id.IsValid())
		asset_manager.RemoveMesh(old_spawn_markers_mesh_id);
}

void GameplaySceneEditor::show_bounds(SceneRenderer & renderer, bool show)
{
	show_polygon_overlay(renderer, m_bounds_overlay, show);
}

void GameplaySceneEditor::show_scent_trail(SceneRenderer & renderer, bool show)
{
	show_polygon_overlay(renderer, m_scent_trail_overlay, show);
}

void GameplaySceneEditor::rebuild_selected_vertex_marker(AssetManager & asset_manager, SceneRenderer & renderer)
{
	MeshId<Vertex2d> old_mesh_id = m_selected_vertex_marker_mesh_id;
	m_selected_vertex_marker_mesh_id = create_line_mesh(asset_manager, create_selected_vertex_marker_lines());

	RenderObject * selected_marker_ro = renderer.GetRenderObject(m_selected_vertex_marker_ro_id);
	if (selected_marker_ro)
		selected_marker_ro->SetMeshId(m_selected_vertex_marker_mesh_id);

	if (old_mesh_id.IsValid())
		asset_manager.RemoveMesh(old_mesh_id);
}

void GameplaySceneEditor::show_selected_vertex_marker(SceneRenderer & renderer, bool show)
{
	const bool has_selected_vertex = m_is_editing_polygon
		&& m_edit_target
		&& m_selected_vertex_index
		&& *m_selected_vertex_index < m_draft_vertices.size();
	renderer.Show(m_selected_vertex_marker_ro_id, show && has_selected_vertex);
}

void GameplaySceneEditor::show_scene_link_triggers(SceneRenderer & renderer, bool show)
{
	for (std::size_t i = 0; i < m_scene_link_overlays.size(); ++i)
	{
		bool const has_scene = m_scene_data && i < m_scene_data->scene_links.size();
		show_polygon_overlay(renderer, m_scene_link_overlays[i], show && has_scene);
	}
}

void GameplaySceneEditor::show_spawn_markers(SceneRenderer & renderer, bool show)
{
	renderer.Show(m_spawn_markers_ro_id, show);
}

void GameplaySceneEditor::show_polygon_editing_label(SceneRenderer & renderer, bool show)
{
	renderer.Show(m_polygon_editing_label.GetROId(), show);
}

void GameplaySceneEditor::begin_polygon_editing(AssetManager & asset_manager, SceneRenderer & renderer, PolygonEditTarget target)
{
	if (target.kind == PolygonEditTargetKind::SceneLink && (!m_scene_data || target.link_index >= m_scene_data->scene_links.size()))
		return;

	if (m_is_editing_spawn)
		cancel_spawn_editing(asset_manager, renderer);
	if (m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);

	m_is_editing_polygon = true;
	m_edit_target = target;
	m_dragged_vertex_index.reset();
	m_draft_vertices = get_target_vertices(target);
	if (target.kind == PolygonEditTargetKind::SceneLink)
	{
		m_selected_scene_link_index = target.link_index;
		rebuild_spawn_markers(asset_manager, renderer);
	}
	m_selected_vertex_index = m_draft_vertices.empty()
		? std::nullopt
		: std::optional<std::size_t>{ m_draft_vertices.size() - 1 };

	if (target.kind == PolygonEditTargetKind::SceneBounds)
		rebuild_bounds_overlay(asset_manager, renderer, m_draft_vertices, m_draft_vertices.size() >= 3);
	else if (target.kind == PolygonEditTargetKind::ScentTrail)
		rebuild_scent_trail_overlay(asset_manager, renderer, m_draft_vertices);
	else
		rebuild_scene_link_overlay(asset_manager, renderer, target.link_index);

	update_polygon_editing_label();
	rebuild_selected_vertex_marker(asset_manager, renderer);
	show_bounds(renderer, true);
	show_scent_trail(renderer, true);
	show_selected_vertex_marker(renderer, true);
	show_scene_link_triggers(renderer, true);
	show_spawn_markers(renderer, true);
	show_polygon_editing_label(renderer, true);
}

void GameplaySceneEditor::cancel_polygon_editing(AssetManager & asset_manager, SceneRenderer & renderer)
{
	std::optional<PolygonEditTarget> old_target = m_edit_target;
	m_is_editing_polygon = false;
	m_edit_target.reset();
	m_selected_vertex_index.reset();
	m_dragged_vertex_index.reset();
	m_draft_vertices.clear();

	if (m_scene_data)
		rebuild_bounds_overlay(asset_manager, renderer, m_scene_data->bounds.GetVertices(), m_scene_data->bounds.IsValid());
	if (m_scene_data)
		rebuild_scent_trail_overlay(asset_manager, renderer, m_scene_data->scent_trail.points);
	if (old_target && old_target->kind == PolygonEditTargetKind::SceneLink)
		rebuild_scene_link_overlay(asset_manager, renderer, old_target->link_index);

	rebuild_selected_vertex_marker(asset_manager, renderer);
	show_bounds(renderer, true);
	show_scent_trail(renderer, true);
	show_selected_vertex_marker(renderer, true);
	show_scene_link_triggers(renderer, true);
	show_spawn_markers(renderer, true);
	update_polygon_editing_label();
	show_polygon_editing_label(renderer, true);
}

bool GameplaySceneEditor::apply_polygon_draft(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data || !m_edit_target)
		return false;

	const std::size_t min_vertices = m_edit_target->kind == PolygonEditTargetKind::ScentTrail ? 2 : 3;
	if (m_draft_vertices.size() < min_vertices)
		return false;

	PolygonEditTarget const target = *m_edit_target;
	set_target_vertices(target, m_draft_vertices);
	if (target.kind == PolygonEditTargetKind::ScentTrail)
		m_scent_trail_changed = true;

	m_is_editing_polygon = false;
	m_edit_target.reset();
	m_selected_vertex_index.reset();
	m_dragged_vertex_index.reset();
	m_draft_vertices.clear();

	rebuild_bounds_overlay(asset_manager, renderer, m_scene_data->bounds.GetVertices(), m_scene_data->bounds.IsValid());
	rebuild_scent_trail_overlay(asset_manager, renderer, m_scene_data->scent_trail.points);
	rebuild_selected_vertex_marker(asset_manager, renderer);
	rebuild_scene_link_overlays(asset_manager, renderer);
	show_bounds(renderer, true);
	show_scent_trail(renderer, true);
	show_selected_vertex_marker(renderer, true);
	show_scene_link_triggers(renderer, true);
	show_spawn_markers(renderer, true);
	update_polygon_editing_label();
	show_polygon_editing_label(renderer, true);
	return true;
}

void GameplaySceneEditor::add_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 vertex)
{
	if (!m_edit_target)
		return;

	std::size_t insert_index = m_draft_vertices.size();
	if (m_selected_vertex_index && *m_selected_vertex_index < m_draft_vertices.size())
		insert_index = *m_selected_vertex_index + 1;

	m_draft_vertices.insert(m_draft_vertices.begin() + static_cast<std::ptrdiff_t>(insert_index), vertex);
	m_selected_vertex_index = insert_index;

	rebuild_edit_target_overlay(asset_manager, renderer);
	rebuild_selected_vertex_marker(asset_manager, renderer);
	show_selected_vertex_marker(renderer, true);
	update_polygon_editing_label();
}

void GameplaySceneEditor::remove_last_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_edit_target || m_draft_vertices.empty())
		return;

	m_draft_vertices.pop_back();
	update_selected_vertex_index_after_size_change();

	rebuild_edit_target_overlay(asset_manager, renderer);
	rebuild_selected_vertex_marker(asset_manager, renderer);
	show_selected_vertex_marker(renderer, true);
	update_polygon_editing_label();
}

void GameplaySceneEditor::select_next_polygon_vertex(AssetManager & asset_manager, SceneRenderer & renderer, int direction)
{
	if (!m_edit_target || m_draft_vertices.empty())
		return;

	const int count = static_cast<int>(m_draft_vertices.size());
	int selected_index = m_selected_vertex_index
		? static_cast<int>(*m_selected_vertex_index)
		: direction >= 0 ? -1 : 0;
	selected_index = (selected_index + direction) % count;
	if (selected_index < 0)
		selected_index += count;

	m_selected_vertex_index = static_cast<std::size_t>(selected_index);
	rebuild_selected_vertex_marker(asset_manager, renderer);
	show_selected_vertex_marker(renderer, true);
	update_polygon_editing_label();
}

void GameplaySceneEditor::append_polygon_vertex(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_edit_target)
		return;

	glm::vec2 point{ 0.0f };
	if (m_draft_vertices.empty())
	{
		if (m_scene_data && !m_scene_data->scene_links.empty())
			point = m_scene_data->scene_links.front().dog_arrival_pos;
	}
	else
	{
		std::size_t selected_index = m_draft_vertices.size() - 1;
		if (m_selected_vertex_index && *m_selected_vertex_index < m_draft_vertices.size())
			selected_index = *m_selected_vertex_index;

		bool const is_closed_polygon = m_edit_target->kind != PolygonEditTargetKind::ScentTrail;
		if (selected_index + 1 < m_draft_vertices.size())
		{
			point = (m_draft_vertices[selected_index] + m_draft_vertices[selected_index + 1]) * 0.5f;
		}
		else if (is_closed_polygon && m_draft_vertices.size() > 1)
		{
			point = (m_draft_vertices[selected_index] + m_draft_vertices.front()) * 0.5f;
		}
		else
		{
			glm::vec2 dir{ 0.0f, 1.0f };
			if (selected_index > 0)
				dir = m_draft_vertices[selected_index] - m_draft_vertices[selected_index - 1];

			if (glm::length(dir) < 1e-5f)
				dir = glm::vec2{ 0.0f, 1.0f };
			else
				dir = glm::normalize(dir);

			point = m_draft_vertices[selected_index] + dir * PolygonAppendDistance;
		}
	}

	add_draft_vertex(asset_manager, renderer, point);
}

void GameplaySceneEditor::delete_selected_polygon_vertex(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_edit_target
		|| !m_selected_vertex_index
		|| *m_selected_vertex_index >= m_draft_vertices.size())
	{
		return;
	}

	m_draft_vertices.erase(m_draft_vertices.begin() + static_cast<std::ptrdiff_t>(*m_selected_vertex_index));
	m_dragged_vertex_index.reset();
	update_selected_vertex_index_after_size_change();
	rebuild_edit_target_overlay(asset_manager, renderer);
	rebuild_selected_vertex_marker(asset_manager, renderer);
	show_selected_vertex_marker(renderer, true);
	update_polygon_editing_label();
}

void GameplaySceneEditor::nudge_selected_polygon_vertex(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 delta)
{
	if (!m_edit_target
		|| !m_selected_vertex_index
		|| *m_selected_vertex_index >= m_draft_vertices.size())
	{
		return;
	}

	m_draft_vertices[*m_selected_vertex_index] += delta;
	rebuild_edit_target_overlay(asset_manager, renderer);
	rebuild_selected_vertex_marker(asset_manager, renderer);
	show_selected_vertex_marker(renderer, true);
	update_polygon_editing_label();
}

void GameplaySceneEditor::move_selected_polygon_vertex_to(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 position)
{
	if (!m_edit_target
		|| !m_selected_vertex_index
		|| *m_selected_vertex_index >= m_draft_vertices.size())
	{
		return;
	}

	m_draft_vertices[*m_selected_vertex_index] = position;
	rebuild_edit_target_overlay(asset_manager, renderer);
	rebuild_selected_vertex_marker(asset_manager, renderer);
	show_selected_vertex_marker(renderer, true);
	update_polygon_editing_label();
}

std::optional<std::size_t> GameplaySceneEditor::find_nearest_draft_vertex(glm::vec2 point, float max_distance) const
{
	std::optional<std::size_t> nearest_index;
	float nearest_distance_sq = max_distance * max_distance;
	for (std::size_t i = 0; i < m_draft_vertices.size(); ++i)
	{
		glm::vec2 const offset = m_draft_vertices[i] - point;
		float const distance_sq = glm::dot(offset, offset);
		if (distance_sq <= nearest_distance_sq)
		{
			nearest_distance_sq = distance_sq;
			nearest_index = i;
		}
	}

	return nearest_index;
}

void GameplaySceneEditor::update_selected_vertex_index_after_size_change()
{
	if (m_draft_vertices.empty())
	{
		m_selected_vertex_index.reset();
		return;
	}

	if (!m_selected_vertex_index || *m_selected_vertex_index >= m_draft_vertices.size())
		m_selected_vertex_index = m_draft_vertices.size() - 1;
}

void GameplaySceneEditor::rebuild_edit_target_overlay(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_edit_target)
		return;

	if (m_edit_target->kind == PolygonEditTargetKind::SceneBounds)
		rebuild_bounds_overlay(asset_manager, renderer, m_draft_vertices, m_draft_vertices.size() >= 3);
	else if (m_edit_target->kind == PolygonEditTargetKind::ScentTrail)
		rebuild_scent_trail_overlay(asset_manager, renderer, m_draft_vertices);
	else
		rebuild_scene_link_overlay(asset_manager, renderer, m_edit_target->link_index);
}

void GameplaySceneEditor::begin_spawn_editing(AssetManager & asset_manager, SceneRenderer & renderer, SpawnEditTarget target)
{
	if (!m_scene_data || target.link_index >= m_scene_data->scene_links.size())
		return;

	if (m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);
	if (m_is_editing_spawn)
		cancel_spawn_editing(asset_manager, renderer);

	m_is_editing_spawn = true;
	m_spawn_edit_target = target;
	m_selected_scene_link_index = target.link_index;

	rebuild_spawn_markers(asset_manager, renderer);
	update_polygon_editing_label();
	show_spawn_markers(renderer, true);
	show_polygon_editing_label(renderer, true);
}

void GameplaySceneEditor::cancel_spawn_editing(AssetManager & asset_manager, SceneRenderer & renderer)
{
	m_is_editing_spawn = false;
	m_spawn_edit_target.reset();
	rebuild_spawn_markers(asset_manager, renderer);
	update_polygon_editing_label();
	show_spawn_markers(renderer, true);
	show_polygon_editing_label(renderer, true);
}

void GameplaySceneEditor::set_spawn_position(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 pos)
{
	if (!m_spawn_edit_target)
		return;

	set_spawn_position(*m_spawn_edit_target, pos);
	m_is_editing_spawn = false;
	m_spawn_edit_target.reset();
	rebuild_spawn_markers(asset_manager, renderer);
	update_polygon_editing_label();
	show_spawn_markers(renderer, true);
	show_polygon_editing_label(renderer, true);
}

void GameplaySceneEditor::select_next_scene_link(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data || m_scene_data->scene_links.empty())
	{
		m_selected_scene_link_index.reset();
		return;
	}

	if (m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);
	if (m_is_editing_spawn)
		cancel_spawn_editing(asset_manager, renderer);

	if (!m_selected_scene_link_index)
		m_selected_scene_link_index = 0;
	else
		m_selected_scene_link_index = (*m_selected_scene_link_index + 1) % m_scene_data->scene_links.size();

	rebuild_scene_link_overlays(asset_manager, renderer);
	rebuild_spawn_markers(asset_manager, renderer);
	update_polygon_editing_label();
}

void GameplaySceneEditor::create_scene_link(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data)
		return;

	if (m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);
	if (m_is_editing_spawn)
		cancel_spawn_editing(asset_manager, renderer);

	m_scene_data->scene_links.emplace_back();
	m_selected_scene_link_index = m_scene_data->scene_links.size() - 1;
	rebuild_scene_link_overlays(asset_manager, renderer);
	rebuild_spawn_markers(asset_manager, renderer);
	begin_polygon_editing(asset_manager, renderer, selected_scene_link_target());
}

void GameplaySceneEditor::delete_selected_scene_link(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!has_selected_scene_link())
		return;

	if (m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);
	if (m_is_editing_spawn)
		cancel_spawn_editing(asset_manager, renderer);

	std::size_t const erased_index = *m_selected_scene_link_index;
	m_scene_data->scene_links.erase(m_scene_data->scene_links.begin() + static_cast<std::ptrdiff_t>(erased_index));
	if (m_scene_data->scene_links.empty())
		m_selected_scene_link_index.reset();
	else if (erased_index >= m_scene_data->scene_links.size())
		m_selected_scene_link_index = m_scene_data->scene_links.size() - 1;

	rebuild_scene_link_overlays(asset_manager, renderer);
	rebuild_spawn_markers(asset_manager, renderer);
	update_polygon_editing_label();
}

bool GameplaySceneEditor::has_selected_scene_link() const
{
	return m_scene_data
		&& m_selected_scene_link_index
		&& *m_selected_scene_link_index < m_scene_data->scene_links.size();
}

GameplaySceneEditor::PolygonEditTarget GameplaySceneEditor::selected_scene_link_target() const
{
	return PolygonEditTarget{
		.kind = PolygonEditTargetKind::SceneLink,
		.link_index = m_selected_scene_link_index.value_or(0)
	};
}

GameplaySceneEditor::SpawnEditTarget GameplaySceneEditor::selected_scene_link_spawn_target(SpawnCharacter character) const
{
	return SpawnEditTarget{
		.character = character,
		.link_index = m_selected_scene_link_index.value_or(0)
	};
}

std::vector<glm::vec2> GameplaySceneEditor::get_target_vertices(PolygonEditTarget target) const
{
	if (!m_scene_data)
		return {};

	if (target.kind == PolygonEditTargetKind::SceneBounds)
		return m_scene_data->bounds.GetVertices();
	if (target.kind == PolygonEditTargetKind::ScentTrail)
		return m_scene_data->scent_trail.points;
	if (target.link_index < m_scene_data->scene_links.size())
		return m_scene_data->scene_links[target.link_index].trigger.GetVertices();
	return {};
}

void GameplaySceneEditor::set_target_vertices(PolygonEditTarget target, std::vector<glm::vec2> vertices)
{
	if (!m_scene_data)
		return;

	if (target.kind == PolygonEditTargetKind::SceneBounds)
		m_scene_data->bounds.SetVertices(std::move(vertices));
	else if (target.kind == PolygonEditTargetKind::ScentTrail)
		m_scene_data->scent_trail.points = std::move(vertices);
	else if (target.link_index < m_scene_data->scene_links.size())
		m_scene_data->scene_links[target.link_index].trigger.SetVertices(std::move(vertices));
}

glm::vec2 GameplaySceneEditor::get_spawn_position(SpawnEditTarget target) const
{
	if (!m_scene_data)
		return glm::vec2{ 0.0f };

	if (target.link_index >= m_scene_data->scene_links.size())
		return glm::vec2{ 0.0f };

	GameplaySceneLink const & scene_link = m_scene_data->scene_links[target.link_index];
	return target.character == SpawnCharacter::Dog
		? scene_link.dog_arrival_pos
		: scene_link.baby_arrival_pos;
}

void GameplaySceneEditor::set_spawn_position(SpawnEditTarget target, glm::vec2 pos)
{
	if (!m_scene_data)
		return;

	if (target.link_index >= m_scene_data->scene_links.size())
		return;

	GameplaySceneLink & scene_link = m_scene_data->scene_links[target.link_index];
	if (target.character == SpawnCharacter::Dog)
		scene_link.dog_arrival_pos = pos;
	else
		scene_link.baby_arrival_pos = pos;
}

void GameplaySceneEditor::update_polygon_editing_label()
{
	m_polygon_editing_label.SetText(create_editor_label_text());
}

std::string GameplaySceneEditor::create_editor_label_text() const
{
	if (m_is_editing_spawn && m_spawn_edit_target)
	{
		std::string owner_text = "link";
		if (m_scene_data && m_spawn_edit_target->link_index < m_scene_data->scene_links.size())
		{
			GameplaySceneLink const & scene_link = m_scene_data->scene_links[m_spawn_edit_target->link_index];
			owner_text = "link #" + std::to_string(m_spawn_edit_target->link_index + 1)
				+ ": " + std::string{ ToString(scene_link.target_scene_id) };
		}

		std::string const character_text = m_spawn_edit_target->character == SpawnCharacter::Dog ? "dog" : "baby";
		return "Editing " + owner_text + " " + character_text + " arrival point\n"
			"[Left Click] Place point\n"
			"[Escape] Cancel";
	}

	if (!m_is_editing_polygon || !m_edit_target)
	{
		std::string selected_scene_link_text = "none";
		if (has_selected_scene_link())
		{
			GameplaySceneLink const & scene_link = m_scene_data->scene_links[*m_selected_scene_link_index];
			selected_scene_link_text = "#" + std::to_string(*m_selected_scene_link_index + 1)
				+ ": " + std::string{ ToString(scene_link.target_scene_id) };
		}

		return "Editing\n"
			"[B] Edit bounds\n"
			"[T] Edit scent trail\n"
			"[Tab] Select link (" + selected_scene_link_text + ")\n"
			"[L] Edit selected link trigger\n"
			"[Shift+L] New link\n"
			"[Delete] Delete selected link\n"
			"[1] Dog selected link arrival point\n"
			"[2] Baby selected link arrival point\n"
			"[Ctrl+S] Save\n"
			"[R] Reload\n"
			"[Escape] Exit editor";
	}

	if (m_edit_target->kind == PolygonEditTargetKind::SceneBounds)
	{
		std::string selected_vertex_text = "none";
		if (m_selected_vertex_index && *m_selected_vertex_index < m_draft_vertices.size())
			selected_vertex_text = std::to_string(*m_selected_vertex_index + 1)
				+ "/" + std::to_string(m_draft_vertices.size());

		return "Editing scene bounds\n"
			"Selected vertex: " + selected_vertex_text + "\n"
			"[Left Drag] Move vertex\n"
			"[N] Insert vertex after selected\n"
			"[[ or ]] Select vertex\n"
			"[WASD]/[Arrows] Nudge vertex\n"
			"[Shift] Coarse, [Ctrl] Fine\n"
			"[Delete] Delete vertex\n"
			"[Enter] Apply bounds\n"
			"[Escape] Cancel";
	}

	if (m_edit_target->kind == PolygonEditTargetKind::ScentTrail)
	{
		std::string selected_point_text = "none";
		if (m_selected_vertex_index && *m_selected_vertex_index < m_draft_vertices.size())
			selected_point_text = std::to_string(*m_selected_vertex_index + 1)
				+ "/" + std::to_string(m_draft_vertices.size());

		return "Editing scent trail\n"
			"Selected point: " + selected_point_text + "\n"
			"[Left Drag] Move point\n"
			"[N] Insert point after selected\n"
			"[[ or ]] Select point\n"
			"[WASD]/[Arrows] Nudge point\n"
			"[Shift] Coarse, [Ctrl] Fine\n"
			"[Delete] Delete point\n"
			"[Enter] Apply trail\n"
			"[Escape] Cancel";
	}

	if (!m_scene_data || m_edit_target->link_index >= m_scene_data->scene_links.size())
	{
		std::string selected_vertex_text = "none";
		if (m_selected_vertex_index && *m_selected_vertex_index < m_draft_vertices.size())
			selected_vertex_text = std::to_string(*m_selected_vertex_index + 1)
				+ "/" + std::to_string(m_draft_vertices.size());

		return "Editing link trigger\n"
			"Selected vertex: " + selected_vertex_text + "\n"
			"[Left Drag] Move vertex\n"
			"[N] Insert vertex after selected\n"
			"[[ or ]] Select vertex\n"
			"[WASD]/[Arrows] Nudge vertex\n"
			"[Shift] Coarse, [Ctrl] Fine\n"
			"[Delete] Delete vertex\n"
			"[Enter] Apply trigger\n"
			"[Escape] Cancel";
	}

	GameplaySceneLink const & scene_link = m_scene_data->scene_links[m_edit_target->link_index];
	std::string selected_vertex_text = "none";
	if (m_selected_vertex_index && *m_selected_vertex_index < m_draft_vertices.size())
		selected_vertex_text = std::to_string(*m_selected_vertex_index + 1)
			+ "/" + std::to_string(m_draft_vertices.size());

	return "Editing link #" + std::to_string(m_edit_target->link_index + 1)
		+ ": " + std::string{ ToString(scene_link.target_scene_id) } + "\n"
		"Selected vertex: " + selected_vertex_text + "\n"
		"[Left Drag] Move vertex\n"
		"[N] Insert vertex after selected\n"
		"[[ or ]] Select vertex\n"
		"[WASD]/[Arrows] Nudge vertex\n"
		"[Shift] Coarse, [Ctrl] Fine\n"
		"[Delete] Delete vertex\n"
		"[Enter] Apply trigger\n"
		"[Escape] Cancel";
}

bool GameplaySceneEditor::save_scene_data() const
{
	return m_scene_data && GameplaySceneLoader::SaveSceneData(m_scene_filepath, *m_scene_data);
}

bool GameplaySceneEditor::ConsumeScentTrailChanged()
{
	bool const changed = m_scent_trail_changed;
	m_scent_trail_changed = false;
	return changed;
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

std::vector<LineInstance> GameplaySceneEditor::create_edge_lines(
	std::vector<glm::vec2> const & vertices,
	bool close_edges,
	glm::vec4 color,
	float thickness) const
{
	std::vector<LineInstance> lines;

	const std::size_t edge_count = close_edges
		? vertices.size()
		: vertices.size() > 1 ? vertices.size() - 1 : 0;
	if (edge_count == 0)
		return lines;

	lines.reserve(edge_count);
	for (std::size_t i = 0; i < edge_count; ++i)
	{
		glm::vec2 const & a = vertices[i];
		glm::vec2 const & b = vertices[(i + 1) % vertices.size()];
		lines.push_back(LineInstance{
			.p0 = { a.x, a.y, 0.0f },
			.p1 = { b.x, b.y, 0.0f },
			.thickness = thickness,
			.color = color
		});
	}

	return lines;
}

std::vector<LineInstance> GameplaySceneEditor::create_spawn_marker_lines() const
{
	std::vector<LineInstance> lines;
	if (!m_scene_data)
		return lines;

	auto append_marker = [&](SpawnEditTarget target, glm::vec4 base_color)
	{
		bool const is_selected_scene_link = m_selected_scene_link_index
			&& *m_selected_scene_link_index == target.link_index;
		bool const is_editing_this = m_is_editing_spawn
			&& m_spawn_edit_target
			&& m_spawn_edit_target->character == target.character
			&& m_spawn_edit_target->link_index == target.link_index;

		glm::vec4 const color = is_editing_this
			? EditingSpawnColor
			: is_selected_scene_link ? SelectedSpawnColor : base_color;
		float const size = is_editing_this || is_selected_scene_link ? SelectedSpawnMarkerSize : SpawnMarkerSize;
		float const thickness = is_editing_this || is_selected_scene_link ? SelectedSpawnMarkerThickness : SpawnMarkerThickness;
		append_spawn_marker_lines(lines, get_spawn_position(target), color, size, thickness);
	};

	for (std::size_t i = 0; i < m_scene_data->scene_links.size(); ++i)
	{
		append_marker(SpawnEditTarget{
			.character = SpawnCharacter::Dog,
			.link_index = i
		}, LinkDogArrivalColor);
		append_marker(SpawnEditTarget{
			.character = SpawnCharacter::Baby,
			.link_index = i
		}, LinkBabyArrivalColor);
	}

	return lines;
}

std::vector<LineInstance> GameplaySceneEditor::create_selected_vertex_marker_lines() const
{
	std::vector<LineInstance> lines;
	if (!m_selected_vertex_index || *m_selected_vertex_index >= m_draft_vertices.size())
		return lines;

	append_spawn_marker_lines(
		lines,
		m_draft_vertices[*m_selected_vertex_index],
		SelectedVertexColor,
		SelectedVertexMarkerSize,
		SelectedVertexMarkerThickness);

	return lines;
}

void GameplaySceneEditor::append_spawn_marker_lines(
	std::vector<LineInstance> & lines,
	glm::vec2 pos,
	glm::vec4 color,
	float size,
	float thickness) const
{
	const float z = 0.02f;
	lines.push_back(LineInstance{
		.p0 = { pos.x - size, pos.y, z },
		.p1 = { pos.x + size, pos.y, z },
		.thickness = thickness,
		.color = color
	});
	lines.push_back(LineInstance{
		.p0 = { pos.x, pos.y - size, z },
		.p1 = { pos.x, pos.y + size, z },
		.thickness = thickness,
		.color = color
	});
	lines.push_back(LineInstance{
		.p0 = { pos.x - size * 0.7f, pos.y - size * 0.7f, z },
		.p1 = { pos.x + size * 0.7f, pos.y + size * 0.7f, z },
		.thickness = thickness * 0.7f,
		.color = color
	});
	lines.push_back(LineInstance{
		.p0 = { pos.x - size * 0.7f, pos.y + size * 0.7f, z },
		.p1 = { pos.x + size * 0.7f, pos.y - size * 0.7f, z },
		.thickness = thickness * 0.7f,
		.color = color
	});
}

std::vector<LineInstance> GameplaySceneEditor::create_point_lines(
	std::vector<glm::vec2> const & vertices,
	glm::vec4 color,
	float thickness) const
{
	std::vector<LineInstance> lines;
	lines.reserve(vertices.size() * 2);

	constexpr float point_radius = 0.05f;

	for (glm::vec2 const & vertex : vertices)
	{
		lines.push_back(LineInstance{
			.p0 = { vertex.x - point_radius, vertex.y, 0.0f },
			.p1 = { vertex.x + point_radius, vertex.y, 0.0f },
			.thickness = thickness,
			.color = color
		});
		lines.push_back(LineInstance{
			.p0 = { vertex.x, vertex.y - point_radius, 0.0f },
			.p1 = { vertex.x, vertex.y + point_radius, 0.0f },
			.thickness = thickness,
			.color = color
		});
	}

	return lines;
}
