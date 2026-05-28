// SceneForestPath.ixx

module;

#include <filesystem>
#include <memory>
#include <optional>

#include <glm/glm.hpp>

export module SceneForestPath;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import AssetPool;
import Baby;
import Background;
import BackgroundTexPipeline;
import Camera;
import ColorPipeline;
import Dog;
import EditorGrid;
import FontAtlas;
import FPSLabel;
import Input;
import IScene;
import LinePipeline;
import Polygon2d;
import SceneRenderer;
import SniffTheWayConstants;
import SpritePipeline;
import TextPipeline;
import UILabel;
import UIShadow;
import Vertex;

using namespace SniffTheWay;

export class SceneForestPath : public IScene
{
public:
	explicit SceneForestPath(dh::RenderContext const & render_context);

	void OnViewportResized(int width, int height) override;
	void OnDPIScaleFactorChanged(float dpi_scale_factor) override;

	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void Render() const override;

	void ChangeSceneState(SceneState new_state);

private:
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera3d m_camera3d;
	Camera2d m_camera2d;
	SceneState m_scene_state = SceneState::Paused;

	Background m_background;
	Polygon2d m_bounds;
	EditorGrid m_grid;
	Dog m_dog;
	Baby m_baby;

	std::unique_ptr<FontAtlas> m_arial_font;
	FPSLabel m_fps_label;
	std::unique_ptr<UILabel> m_title_label;
	std::unique_ptr<UILabel> m_story_label;
	UIShadow m_story_shadow;
};

SceneForestPath::SceneForestPath(dh::RenderContext const & render_context)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera3d{ render_context.ShouldFlipScreenY() }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
{
	const glm::vec3 camera_pos{ 0.0f, -6.0f, 1.0f };
	const glm::vec3 camera_dir = glm::normalize(glm::vec3{ 0.0f, 5.0f, 0.0f } - camera_pos);
	m_camera3d.Init(camera_pos, camera_dir);

	const auto bg_pipeline_id = m_asset_manager.AddPipeline<BackgroundTexPipeline>(m_camera2d, m_asset_manager);
	const auto line_pipeline_id = m_asset_manager.AddPipeline<LinePipeline>(m_camera3d);
	const auto sprite_pipeline_id = m_asset_manager.AddPipeline<SpritePipeline>(m_camera3d, m_asset_manager);
	const auto color_pipeline_id = m_asset_manager.AddPipeline<ColorPipeline>(m_camera2d);
	const auto text_pipeline_id = m_asset_manager.AddPipeline<TextPipeline>(m_camera2d, m_asset_manager);

	// background
	AssetId bg_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "forest_path.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	m_background.Init(m_asset_manager, bg_tex_id);
	m_renderer.CreateRenderObject("background", m_background.GetMeshId(), bg_pipeline_id, m_background.GetPipelineData());

	m_bounds.SetVertices({
		{-0.5f, -4.0f},
		{0.5f, -4.0f},
		{1.0f, 10.0f},
		{-1.0f, 10.0f},
	});

	// editor grid
	m_grid.Init(m_asset_manager);
	m_grid.SetROId(m_renderer.CreateRenderObject("grid", m_grid.GetMeshId(), line_pipeline_id));

	// dog
	m_dog.Init(m_asset_manager, camera_dir);
	m_renderer.CreateRenderObject("dog", m_dog.GetMeshId(), sprite_pipeline_id, m_dog.GetPipelineData());

	// baby
	m_baby.Init(m_asset_manager, camera_dir);
	m_renderer.CreateRenderObject("baby", m_baby.GetMeshId(), sprite_pipeline_id, m_baby.GetPipelineData());

	// ui
	AssetId arial_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetFontsPath() / "ArialAtlas.png",
		dh::PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_arial_font = std::make_unique<FontAtlas>(arial_tex_id, m_asset_manager.GetFontsPath() / "ArialAtlas.json");

	m_story_shadow.Init(m_asset_manager, -1.0 /*left*/, 1.0 /*right*/, -0.6 /*top*/, -1.0 /*bottom*/);
	m_story_shadow.SetROId(m_renderer.CreateRenderObject("story shadow", m_story_shadow.GetMeshId(), color_pipeline_id));

	m_fps_label.Init(m_asset_manager, *m_arial_font);
	m_renderer.CreateRenderObject("fps label", m_fps_label.GetUILabel()->GetMeshId(), text_pipeline_id, m_fps_label.GetUILabel()->GetPipelineData());

	m_title_label = std::make_unique<UILabel>(m_asset_manager, ShortTitle, *m_arial_font,
		TitleFontSize, glm::vec2{ -0.9, 0.8 } /*origin*/, UILabel::Align::Left, StoryTextColor);
	m_renderer.CreateRenderObject("title", m_title_label->GetMeshId(), text_pipeline_id, m_title_label->GetPipelineData());

	m_story_label = std::make_unique<UILabel>(m_asset_manager, "(Press [Space] to continue)", *m_arial_font,
		LabelFontSize, glm::vec2{ 0.0, -0.8 } /*origin*/, UILabel::Align::Center, StoryTextColor);
	m_story_label->SetROId(m_renderer.CreateRenderObject("story label", m_story_label->GetMeshId(), text_pipeline_id, m_story_label->GetPipelineData()));

	ChangeSceneState(SceneState::Story);
}

// override
void SceneForestPath::OnViewportResized(int width, int height)
{
	m_camera3d.OnViewportResized(width, height);
	m_camera2d.OnViewportResized(width, height);

	// keep UI elements proportional to the height of the view
	m_background.OnViewportResized(width, height, m_asset_manager);
	m_fps_label.OnViewportResized(width, height);
	if (m_title_label)
		m_title_label->OnViewportResized(width, height);
	if (m_story_label)
		m_story_label->OnViewportResized(width, height);
}

// override
void SceneForestPath::OnDPIScaleFactorChanged(float dpi_scale_factor)
{
	m_fps_label.OnDPIScaleFactorChanged(dpi_scale_factor);
	if (m_title_label)
		m_title_label->SetFontSize(TitleFontSize * dpi_scale_factor);
	if (m_story_label)
		m_story_label->SetFontSize(LabelFontSize * dpi_scale_factor);
}

// override
std::optional<SceneTransition> SceneForestPath::Update(float dt, Input const & input)
{
	if (input.KeyJustPressed(Input::Key::Esc))
		return SceneTransition{ SceneId::Exit };

	if (m_scene_state == SceneState::Story && input.KeyJustPressed(Input::Key::Space))
	{
		ChangeSceneState(SceneState::Gameplay);
		m_renderer.Show(m_story_label->GetROId(), false);
		m_renderer.Show(m_story_shadow.GetROId(), false);
	}

	m_camera3d.Update(dt, input);
	m_fps_label.Update(dt);
	m_grid.Update(input, m_renderer, m_scene_state);
	m_dog.Update(dt, input, m_bounds, m_scene_state);
	m_baby.Update(dt, &m_dog, m_scene_state);

	if (m_scene_state == SceneState::Gameplay && m_dog.GetPipelineData().model[3].y < -3.75)
		return SceneTransition{ SceneId::ForestIntersection };

	return std::nullopt;
}

// override
void SceneForestPath::Render() const
{
	m_renderer.Render();
}

void SceneForestPath::ChangeSceneState(SceneState new_state)
{
	m_scene_state = new_state;
	m_grid.OnSceneStateChanged(m_scene_state, m_renderer);
	m_dog.OnSceneStateChanged(m_scene_state);
	m_baby.OnSceneStateChanged(m_scene_state);
}
