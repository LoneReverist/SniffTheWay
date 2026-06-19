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
	void OnSceneStateChanged(SceneState new_state, AssetManager & asset_manager, SceneRenderer & renderer);
	void Reload(AssetManager & asset_manager, SceneRenderer & renderer);

private:
	enum class PolygonEditTargetKind
	{
		SceneBounds,
		AdjacentScene,
	};

	struct PolygonEditTarget
	{
		PolygonEditTargetKind kind = PolygonEditTargetKind::SceneBounds;
		std::size_t adjacent_index = 0;
	};

	struct PolygonOverlay
	{
		MeshId<Vertex2d> edges_mesh_id;
		MeshId<Vertex2d> points_mesh_id;
		AssetId edges_ro_id;
		AssetId points_ro_id;
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
	void rebuild_adjacent_overlays(AssetManager & asset_manager, SceneRenderer & renderer);
	void rebuild_adjacent_overlay(AssetManager & asset_manager, SceneRenderer & renderer, std::size_t index);
	void show_bounds(SceneRenderer & renderer, bool show);
	void show_adjacent_bounds(SceneRenderer & renderer, bool show);
	void show_polygon_editing_label(SceneRenderer & renderer, bool show);
	void begin_polygon_editing(AssetManager & asset_manager, SceneRenderer & renderer, PolygonEditTarget target);
	void cancel_polygon_editing(AssetManager & asset_manager, SceneRenderer & renderer);
	bool apply_polygon_draft(AssetManager & asset_manager, SceneRenderer & renderer);
	void add_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 vertex);
	void remove_last_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer);
	void select_next_adjacent_scene(AssetManager & asset_manager, SceneRenderer & renderer);
	void create_adjacent_scene(AssetManager & asset_manager, SceneRenderer & renderer);
	void delete_selected_adjacent_scene(AssetManager & asset_manager, SceneRenderer & renderer);
	bool has_selected_adjacent_scene() const;
	PolygonEditTarget selected_adjacent_target() const;
	std::vector<glm::vec2> get_target_vertices(PolygonEditTarget target) const;
	void set_target_vertices(PolygonEditTarget target, std::vector<glm::vec2> vertices);
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

private:
	GameplaySceneData * m_scene_data = nullptr;
	std::filesystem::path m_scene_filepath;
	EditorGrid m_grid;
	UILabel m_polygon_editing_label;
	PipelineId<LinePipeline> m_line_pipeline_id;
	bool m_is_editing_polygon = false;
	std::optional<PolygonEditTarget> m_edit_target;
	std::optional<std::size_t> m_selected_adjacent_scene_index;
	std::vector<glm::vec2> m_draft_vertices;
	PolygonOverlay m_bounds_overlay;
	std::vector<PolygonOverlay> m_adjacent_scene_overlays;
};

namespace
{
	constexpr float BoundsEdgeThickness = 10.0f;
	constexpr float BoundsPointThickness = 12.0f;
	constexpr float AdjacentEdgeThickness = 8.0f;
	constexpr float AdjacentPointThickness = 10.0f;
	constexpr glm::vec4 BoundsEdgeColor{ 1.0f, 0.82f, 0.18f, 0.9f };
	constexpr glm::vec4 BoundsPointColor{ 1.0f, 0.25f, 0.15f, 1.0f };
	constexpr glm::vec4 AdjacentEdgeColor{ 0.1f, 0.75f, 1.0f, 0.75f };
	constexpr glm::vec4 AdjacentPointColor{ 0.2f, 0.9f, 1.0f, 0.95f };
	constexpr glm::vec4 SelectedAdjacentEdgeColor{ 0.2f, 1.0f, 0.45f, 0.95f };
	constexpr glm::vec4 SelectedAdjacentPointColor{ 0.1f, 1.0f, 0.25f, 1.0f };
	constexpr glm::vec4 DraftEdgeColor{ 1.0f, 0.55f, 0.08f, 0.95f };
	constexpr glm::vec4 DraftPointColor{ 1.0f, 0.15f, 0.1f, 1.0f };
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

	rebuild_adjacent_overlays(asset_manager, renderer);

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
	if (ctrl_is_down && input.KeyJustPressed('S'))
	{
		if (m_is_editing_polygon && m_draft_vertices.size() >= 3)
			apply_polygon_draft(asset_manager, renderer);
		save_scene_data();
		return;
	}

	if (m_is_editing_polygon)
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
			apply_polygon_draft(asset_manager, renderer);
			return;
		}
	}

	if (input.KeyJustPressed('B'))
	{
		if (m_is_editing_polygon && m_edit_target && m_edit_target->kind == PolygonEditTargetKind::SceneBounds)
			cancel_polygon_editing(asset_manager, renderer);
		else
			begin_polygon_editing(asset_manager, renderer, PolygonEditTarget{ .kind = PolygonEditTargetKind::SceneBounds });
		return;
	}

	if (input.KeyJustPressed(Input::Key::Tab))
	{
		select_next_adjacent_scene(asset_manager, renderer);
		return;
	}

	const bool shift_is_down = input.KeyIsDown(Input::Key::LeftShift) || input.KeyIsDown(Input::Key::RightShift);
	if (shift_is_down && input.KeyJustPressed('A'))
	{
		create_adjacent_scene(asset_manager, renderer);
		return;
	}

	if (input.KeyJustPressed('A'))
	{
		if (m_is_editing_polygon && m_edit_target && m_edit_target->kind == PolygonEditTargetKind::AdjacentScene)
			cancel_polygon_editing(asset_manager, renderer);
		else if (has_selected_adjacent_scene())
			begin_polygon_editing(asset_manager, renderer, selected_adjacent_target());
		return;
	}

	if (input.KeyJustPressed(Input::Key::Delete))
	{
		delete_selected_adjacent_scene(asset_manager, renderer);
		return;
	}
}

void GameplaySceneEditor::OnSceneStateChanged(SceneState new_state, AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (new_state == SceneState::Gameplay && m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);

	m_grid.OnSceneStateChanged(new_state, renderer);
	show_bounds(renderer, new_state == SceneState::Editing);
	show_adjacent_bounds(renderer, new_state == SceneState::Editing);
	update_polygon_editing_label();
	show_polygon_editing_label(renderer, new_state == SceneState::Editing);
}

void GameplaySceneEditor::Reload(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data)
		return;

	m_is_editing_polygon = false;
	m_edit_target.reset();
	m_draft_vertices.clear();
	if (m_selected_adjacent_scene_index && *m_selected_adjacent_scene_index >= m_scene_data->adjacent_scenes.size())
		m_selected_adjacent_scene_index.reset();

	rebuild_bounds_overlay(asset_manager, renderer, m_scene_data->bounds.GetVertices(), m_scene_data->bounds.IsValid());
	rebuild_adjacent_overlays(asset_manager, renderer);
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

void GameplaySceneEditor::rebuild_adjacent_overlays(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data)
		return;

	for (std::size_t i = m_scene_data->adjacent_scenes.size(); i < m_adjacent_scene_overlays.size(); ++i)
		show_polygon_overlay(renderer, m_adjacent_scene_overlays[i], false);

	while (m_adjacent_scene_overlays.size() < m_scene_data->adjacent_scenes.size())
	{
		std::size_t const index = m_adjacent_scene_overlays.size();
		GameplayAdjacentScene const & adjacent_scene = m_scene_data->adjacent_scenes[index];
		m_adjacent_scene_overlays.push_back(create_polygon_overlay(
			asset_manager,
			renderer,
			"adjacent scene " + std::to_string(index + 1),
			adjacent_scene.collider.GetVertices(),
			adjacent_scene.collider.IsValid(),
			AdjacentEdgeColor,
			AdjacentPointColor,
			AdjacentEdgeThickness,
			AdjacentPointThickness,
			false));
	}

	for (std::size_t i = 0; i < m_scene_data->adjacent_scenes.size(); ++i)
	{
		rebuild_adjacent_overlay(asset_manager, renderer, i);
		show_polygon_overlay(renderer, m_adjacent_scene_overlays[i], true);
	}
}

void GameplaySceneEditor::rebuild_adjacent_overlay(AssetManager & asset_manager, SceneRenderer & renderer, std::size_t index)
{
	if (!m_scene_data || index >= m_scene_data->adjacent_scenes.size() || index >= m_adjacent_scene_overlays.size())
		return;

	bool const is_selected = m_selected_adjacent_scene_index && *m_selected_adjacent_scene_index == index;
	bool const is_editing_this = m_is_editing_polygon
		&& m_edit_target
		&& m_edit_target->kind == PolygonEditTargetKind::AdjacentScene
		&& m_edit_target->adjacent_index == index;
	glm::vec4 edge_color = is_editing_this ? DraftEdgeColor : is_selected ? SelectedAdjacentEdgeColor : AdjacentEdgeColor;
	glm::vec4 point_color = is_editing_this ? DraftPointColor : is_selected ? SelectedAdjacentPointColor : AdjacentPointColor;

	std::vector<glm::vec2> const & vertices = is_editing_this
		? m_draft_vertices
		: m_scene_data->adjacent_scenes[index].collider.GetVertices();
	bool const close_edges = is_editing_this
		? m_draft_vertices.size() >= 3
		: m_scene_data->adjacent_scenes[index].collider.IsValid();

	rebuild_polygon_overlay(
		asset_manager,
		renderer,
		m_adjacent_scene_overlays[index],
		vertices,
		close_edges,
		edge_color,
		point_color,
		AdjacentEdgeThickness,
		AdjacentPointThickness);
}

void GameplaySceneEditor::show_bounds(SceneRenderer & renderer, bool show)
{
	show_polygon_overlay(renderer, m_bounds_overlay, show);
}

void GameplaySceneEditor::show_adjacent_bounds(SceneRenderer & renderer, bool show)
{
	for (std::size_t i = 0; i < m_adjacent_scene_overlays.size(); ++i)
	{
		bool const has_scene = m_scene_data && i < m_scene_data->adjacent_scenes.size();
		show_polygon_overlay(renderer, m_adjacent_scene_overlays[i], show && has_scene);
	}
}

void GameplaySceneEditor::show_polygon_editing_label(SceneRenderer & renderer, bool show)
{
	renderer.Show(m_polygon_editing_label.GetROId(), show);
}

void GameplaySceneEditor::begin_polygon_editing(AssetManager & asset_manager, SceneRenderer & renderer, PolygonEditTarget target)
{
	if (target.kind == PolygonEditTargetKind::AdjacentScene && (!m_scene_data || target.adjacent_index >= m_scene_data->adjacent_scenes.size()))
		return;

	if (m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);

	m_is_editing_polygon = true;
	m_edit_target = target;
	m_draft_vertices = get_target_vertices(target);
	if (target.kind == PolygonEditTargetKind::AdjacentScene)
		m_selected_adjacent_scene_index = target.adjacent_index;

	if (target.kind == PolygonEditTargetKind::SceneBounds)
		rebuild_bounds_overlay(asset_manager, renderer, m_draft_vertices, m_draft_vertices.size() >= 3);
	else
		rebuild_adjacent_overlay(asset_manager, renderer, target.adjacent_index);

	update_polygon_editing_label();
	show_bounds(renderer, true);
	show_adjacent_bounds(renderer, true);
	show_polygon_editing_label(renderer, true);
}

void GameplaySceneEditor::cancel_polygon_editing(AssetManager & asset_manager, SceneRenderer & renderer)
{
	std::optional<PolygonEditTarget> old_target = m_edit_target;
	m_is_editing_polygon = false;
	m_edit_target.reset();
	m_draft_vertices.clear();

	if (m_scene_data)
		rebuild_bounds_overlay(asset_manager, renderer, m_scene_data->bounds.GetVertices(), m_scene_data->bounds.IsValid());
	if (old_target && old_target->kind == PolygonEditTargetKind::AdjacentScene)
		rebuild_adjacent_overlay(asset_manager, renderer, old_target->adjacent_index);

	show_bounds(renderer, true);
	show_adjacent_bounds(renderer, true);
	update_polygon_editing_label();
	show_polygon_editing_label(renderer, true);
}

bool GameplaySceneEditor::apply_polygon_draft(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data || !m_edit_target || m_draft_vertices.size() < 3)
		return false;

	PolygonEditTarget const target = *m_edit_target;
	set_target_vertices(target, m_draft_vertices);
	m_is_editing_polygon = false;
	m_edit_target.reset();
	m_draft_vertices.clear();

	rebuild_bounds_overlay(asset_manager, renderer, m_scene_data->bounds.GetVertices(), m_scene_data->bounds.IsValid());
	rebuild_adjacent_overlays(asset_manager, renderer);
	show_bounds(renderer, true);
	show_adjacent_bounds(renderer, true);
	update_polygon_editing_label();
	show_polygon_editing_label(renderer, true);
	return true;
}

void GameplaySceneEditor::add_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer, glm::vec2 vertex)
{
	if (!m_edit_target)
		return;

	m_draft_vertices.push_back(vertex);
	if (m_edit_target->kind == PolygonEditTargetKind::SceneBounds)
		rebuild_bounds_overlay(asset_manager, renderer, m_draft_vertices, m_draft_vertices.size() >= 3);
	else
		rebuild_adjacent_overlay(asset_manager, renderer, m_edit_target->adjacent_index);
}

void GameplaySceneEditor::remove_last_draft_vertex(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_edit_target || m_draft_vertices.empty())
		return;

	m_draft_vertices.pop_back();
	if (m_edit_target->kind == PolygonEditTargetKind::SceneBounds)
		rebuild_bounds_overlay(asset_manager, renderer, m_draft_vertices, m_draft_vertices.size() >= 3);
	else
		rebuild_adjacent_overlay(asset_manager, renderer, m_edit_target->adjacent_index);
}

void GameplaySceneEditor::select_next_adjacent_scene(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data || m_scene_data->adjacent_scenes.empty())
	{
		m_selected_adjacent_scene_index.reset();
		return;
	}

	if (m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);

	if (!m_selected_adjacent_scene_index)
		m_selected_adjacent_scene_index = 0;
	else
		m_selected_adjacent_scene_index = (*m_selected_adjacent_scene_index + 1) % m_scene_data->adjacent_scenes.size();

	rebuild_adjacent_overlays(asset_manager, renderer);
	update_polygon_editing_label();
}

void GameplaySceneEditor::create_adjacent_scene(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!m_scene_data)
		return;

	if (m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);

	m_scene_data->adjacent_scenes.emplace_back();
	m_selected_adjacent_scene_index = m_scene_data->adjacent_scenes.size() - 1;
	rebuild_adjacent_overlays(asset_manager, renderer);
	begin_polygon_editing(asset_manager, renderer, selected_adjacent_target());
}

void GameplaySceneEditor::delete_selected_adjacent_scene(AssetManager & asset_manager, SceneRenderer & renderer)
{
	if (!has_selected_adjacent_scene())
		return;

	if (m_is_editing_polygon)
		cancel_polygon_editing(asset_manager, renderer);

	std::size_t const erased_index = *m_selected_adjacent_scene_index;
	m_scene_data->adjacent_scenes.erase(m_scene_data->adjacent_scenes.begin() + static_cast<std::ptrdiff_t>(erased_index));
	if (m_scene_data->adjacent_scenes.empty())
		m_selected_adjacent_scene_index.reset();
	else if (erased_index >= m_scene_data->adjacent_scenes.size())
		m_selected_adjacent_scene_index = m_scene_data->adjacent_scenes.size() - 1;

	rebuild_adjacent_overlays(asset_manager, renderer);
	update_polygon_editing_label();
}

bool GameplaySceneEditor::has_selected_adjacent_scene() const
{
	return m_scene_data
		&& m_selected_adjacent_scene_index
		&& *m_selected_adjacent_scene_index < m_scene_data->adjacent_scenes.size();
}

GameplaySceneEditor::PolygonEditTarget GameplaySceneEditor::selected_adjacent_target() const
{
	return PolygonEditTarget{
		.kind = PolygonEditTargetKind::AdjacentScene,
		.adjacent_index = m_selected_adjacent_scene_index.value_or(0)
	};
}

std::vector<glm::vec2> GameplaySceneEditor::get_target_vertices(PolygonEditTarget target) const
{
	if (!m_scene_data)
		return {};

	if (target.kind == PolygonEditTargetKind::SceneBounds)
		return m_scene_data->bounds.GetVertices();
	if (target.adjacent_index < m_scene_data->adjacent_scenes.size())
		return m_scene_data->adjacent_scenes[target.adjacent_index].collider.GetVertices();
	return {};
}

void GameplaySceneEditor::set_target_vertices(PolygonEditTarget target, std::vector<glm::vec2> vertices)
{
	if (!m_scene_data)
		return;

	if (target.kind == PolygonEditTargetKind::SceneBounds)
		m_scene_data->bounds.SetVertices(std::move(vertices));
	else if (target.adjacent_index < m_scene_data->adjacent_scenes.size())
		m_scene_data->adjacent_scenes[target.adjacent_index].collider.SetVertices(std::move(vertices));
}

void GameplaySceneEditor::update_polygon_editing_label()
{
	m_polygon_editing_label.SetText(create_editor_label_text());
}

std::string GameplaySceneEditor::create_editor_label_text() const
{
	if (!m_is_editing_polygon || !m_edit_target)
	{
		std::string selected_adjacent_text = "none";
		if (has_selected_adjacent_scene())
		{
			GameplayAdjacentScene const & adjacent_scene = m_scene_data->adjacent_scenes[*m_selected_adjacent_scene_index];
			selected_adjacent_text = "#" + std::to_string(*m_selected_adjacent_scene_index + 1)
				+ ": " + std::string{ ToString(adjacent_scene.scene_id) };
		}

		return "Editing\n"
			"[E] Exit editor\n"
			"[B] Edit bounds\n"
			"[Tab] Select adjacent (" + selected_adjacent_text + ")\n"
			"[A] Edit selected adjacent\n"
			"[Shift+A] New adjacent\n"
			"[Delete] Delete selected adjacent\n"
			"[Ctrl+S] Save\n"
			"[R] Reload";
	}

	if (m_edit_target->kind == PolygonEditTargetKind::SceneBounds)
	{
		return "Editing scene bounds\n"
			"[Left Click] Add vertex\n"
			"[Right Click] Undo vertex\n"
			"[Backspace] Undo vertex\n"
			"[Enter] Apply bounds\n"
			"[B] Cancel\n"
			"[Ctrl+S] Apply and save";
	}

	if (!m_scene_data || m_edit_target->adjacent_index >= m_scene_data->adjacent_scenes.size())
	{
		return "Editing adjacent collider\n"
			"[Left Click] Add vertex\n"
			"[Right Click] Undo vertex\n"
			"[Backspace] Undo vertex\n"
			"[Enter] Apply collider\n"
			"[A] Cancel\n"
			"[Ctrl+S] Apply and save";
	}

	GameplayAdjacentScene const & adjacent_scene = m_scene_data->adjacent_scenes[m_edit_target->adjacent_index];
	return "Editing adjacent #" + std::to_string(m_edit_target->adjacent_index + 1)
		+ ": " + std::string{ ToString(adjacent_scene.scene_id) } + "\n"
		"[Left Click] Add vertex\n"
		"[Right Click] Undo vertex\n"
		"[Backspace] Undo vertex\n"
		"[Enter] Apply collider\n"
		"[A] Cancel\n"
		"[Ctrl+S] Apply and save";
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
