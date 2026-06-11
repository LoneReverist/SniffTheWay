// ScenePlayground2.ixx

module;

#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>

#include <glm/glm.hpp>

export module ScenePlayground2;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Baby;
import Background;
import Texture2dPipeline;
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
import UIDarkBackdrop;
import UILabel;
import Vertex;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class ScenePlayground2 : public IScene
{
public:
	explicit ScenePlayground2(dh::RenderContext const & render_context);

	void OnWindowResized(int width, int height) override;

	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void DestroyPendingAssets() const override;
	void Render() const override;

	void ChangeSceneState(SceneState new_state);

private:
	static constexpr std::string_view BackgroundImage = "playground2.png";

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

	std::unique_ptr<FontAtlas> m_font_atlas;
	FPSLabel m_fps_label;
	UILabel m_controls_label;
	UIDarkBackdrop m_story_shadow;
};

ScenePlayground2::ScenePlayground2(dh::RenderContext const & render_context)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera3d{ render_context.ShouldFlipScreenY() }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
{
	const auto bg_pipeline_id = m_asset_manager.AddPipeline<Texture2dPipeline>(m_camera2d, m_asset_manager);
	const auto line_pipeline_id = m_asset_manager.AddPipeline<LinePipeline>(m_camera3d);
	const auto sprite_pipeline_id = m_asset_manager.AddPipeline<SpritePipeline>(m_camera3d, m_asset_manager);
	const auto color_pipeline_id = m_asset_manager.AddPipeline<ColorPipeline>(m_camera2d);
	const auto text_pipeline_id = m_asset_manager.AddPipeline<TextPipeline>(m_camera2d, m_asset_manager);

	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);

	// background
	AssetId bg_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "gameplay_backgrounds" / BackgroundImage,
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	m_background.Init(m_asset_manager, bg_tex_id);
	m_renderer.CreateRenderObject("background", RenderLayer::Background, m_background.GetMeshId(), bg_pipeline_id, m_background.GetPipelineData());

	// 3d game world
	const glm::vec3 camera_pos{ 0.0f, -6.0f, 1.0f };
	const glm::vec3 camera_dir = glm::normalize(glm::vec3{ 0.0f, 5.0f, 0.0f } - camera_pos);
	m_camera3d.Init(camera_pos, camera_dir);

	m_bounds.SetVertices({
		{-2.0f, -4.0f},
		{2.0f, -4.0f},
		{2.0f, -2.0f},
		{-2.0f, -2.0f},
	});

	m_grid.Init(m_asset_manager);
	m_grid.SetROId(m_renderer.CreateRenderObject("grid", RenderLayer::Scene3d, m_grid.GetMeshId(), line_pipeline_id));

	m_dog.Init(m_asset_manager, camera_dir, glm::vec2{ -1.5f, -3.0f });
	m_renderer.CreateRenderObject("dog", RenderLayer::Scene3d, m_dog.GetMeshId(), sprite_pipeline_id, m_dog.GetPipelineData());

	m_baby.Init(m_asset_manager, camera_dir, glm::vec2{ -1.75f, -2.25f });
	m_renderer.CreateRenderObject("baby", RenderLayer::Scene3d, m_baby.GetMeshId(), sprite_pipeline_id, m_baby.GetPipelineData());

	// ui
	AssetId font_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetFontsPath() / "Alice.png",
		dh::PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_font_atlas = std::make_unique<FontAtlas>(font_tex_id, m_asset_manager.GetFontsPath() / "Alice.json");

	m_story_shadow.Init(m_asset_manager, 0 /*left*/, UIWidth /*right*/, UIHeight * 0.75f /*top*/, UIHeight /*bottom*/);
	m_story_shadow.SetROId(m_renderer.CreateRenderObject("story shadow", RenderLayer::UIShadow, m_story_shadow.GetMeshId(), color_pipeline_id));

	m_fps_label.Init(m_asset_manager, m_renderer, m_camera2d, *m_font_atlas);

	m_controls_label.Init(m_asset_manager, "(Press [Space] to continue)", *m_font_atlas,
		LabelFontSize, glm::vec2{ 960, 1026 } /*origin*/, UILabel::Align::Center, StoryTextColor);
	m_controls_label.SetROId(m_renderer.CreateRenderObject("controls label", RenderLayer::UIForeground, m_controls_label.GetMeshId(), text_pipeline_id, m_controls_label.GetPipelineData()));

	ChangeSceneState(SceneState::Gameplay);
}

// override
void ScenePlayground2::OnWindowResized(int width, int height)
{
	int game_width = static_cast<int>(height * 16.0f / 9.0f);
	int x_offset = (width - game_width) / 2;
	m_renderer.SetViewport(x_offset, 0, game_width, height);

	m_camera3d.SetViewportSize(game_width, height);
	m_camera2d.SetViewportSize(game_width, height);
}

// override
std::optional<SceneTransition> ScenePlayground2::Update(float dt, Input const & input)
{
	if (input.KeyJustPressed(Input::Key::Esc))
		return SceneTransition{ SceneId::Exit };

	if (m_scene_state == SceneState::Story && input.KeyJustPressed(Input::Key::Space))
		ChangeSceneState(SceneState::Gameplay);

	m_camera3d.Update(dt, input);
	m_fps_label.Update(dt);
	m_grid.Update(input, m_renderer, m_scene_state);
	m_dog.Update(dt, input, m_bounds, m_scene_state);
	m_baby.Update(dt, &m_dog, m_scene_state);

	if (m_scene_state == SceneState::Gameplay && m_dog.GetPipelineData().model[3].x < -1.75)
		return SceneTransition{ SceneId::Playground };
	if (m_scene_state == SceneState::Gameplay && m_dog.GetPipelineData().model[3].x > 1.75)
		return SceneTransition{ SceneId::Creek };

	return std::nullopt;
}

// override
void ScenePlayground2::DestroyPendingAssets() const
{
	m_asset_manager.DestroyPendingAssets();
}

// override
void ScenePlayground2::Render() const
{
	m_fps_label.RenderOffscreenTexture();
	m_renderer.Render();
}

void ScenePlayground2::ChangeSceneState(SceneState new_state)
{
	m_scene_state = new_state;
	
	m_grid.OnSceneStateChanged(m_scene_state, m_renderer);
	m_dog.OnSceneStateChanged(m_scene_state);
	m_baby.OnSceneStateChanged(m_scene_state);

	if (m_scene_state == SceneState::Gameplay)
	{
		m_renderer.Show(m_controls_label.GetROId(), false);
		m_renderer.Show(m_story_shadow.GetROId(), false);
	}
}
