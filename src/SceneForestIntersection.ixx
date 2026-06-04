// SceneForestIntersection.ixx

module;

#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>

#include <glm/glm.hpp>

export module SceneForestIntersection;

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

export class SceneForestIntersection : public IScene
{
public:
	explicit SceneForestIntersection(dh::RenderContext const & render_context);

	void OnWindowResized(int width, int height) override;

	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void Render() const override;

	void ChangeSceneState(SceneState new_state);

private:
	// Gameplay section 3: Familiar trail → strongest scent
	static constexpr std::string_view BackgroundImage = "forest_intersection.png";
	static constexpr std::array<std::string_view, 4> StoryTexts{
		"Find the way home.",
		// small prompts
		"Closer.",
		"The scent feels warm.",
		"Almost there.",
	};

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
	std::unique_ptr<UILabel> m_controls_label;
	std::unique_ptr<UILabel> m_story_label;
	UIShadow m_story_shadow;
};

SceneForestIntersection::SceneForestIntersection(dh::RenderContext const & render_context)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera3d{ render_context.ShouldFlipScreenY() }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
{
	const auto bg_pipeline_id = m_asset_manager.AddPipeline<BackgroundTexPipeline>(m_camera2d, m_asset_manager);
	const auto line_pipeline_id = m_asset_manager.AddPipeline<LinePipeline>(m_camera3d);
	const auto sprite_pipeline_id = m_asset_manager.AddPipeline<SpritePipeline>(m_camera3d, m_asset_manager);
	const auto color_pipeline_id = m_asset_manager.AddPipeline<ColorPipeline>(m_camera2d);
	const auto text_pipeline_id = m_asset_manager.AddPipeline<TextPipeline>(m_camera2d, m_asset_manager);

	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);

	// background
	AssetId bg_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "gameplay_backgrounds" / BackgroundImage,
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	m_background.Init(m_asset_manager, bg_tex_id);
	m_renderer.CreateRenderObject("background", m_background.GetMeshId(), bg_pipeline_id, m_background.GetPipelineData());

	// 3d game world
	const glm::vec3 camera_pos{ 0.0f, -6.0f, 1.0f };
	const glm::vec3 camera_dir = glm::normalize(glm::vec3{ 0.0f, 5.0f, 0.0f } - camera_pos);
	m_camera3d.Init(camera_pos, camera_dir);

	m_bounds.SetVertices({
		{-0.5f, -4.0f},
		{0.5f, -4.0f},
		{1.0f, 10.0f},
		{-1.0f, 10.0f},
	});

	m_grid.Init(m_asset_manager);
	m_grid.SetROId(m_renderer.CreateRenderObject("grid", m_grid.GetMeshId(), line_pipeline_id));

	m_dog.Init(m_asset_manager, camera_dir, glm::vec2{ 0.0f, 5.0f });
	m_renderer.CreateRenderObject("dog", m_dog.GetMeshId(), sprite_pipeline_id, m_dog.GetPipelineData());

	m_baby.Init(m_asset_manager, camera_dir, glm::vec2{ 0.0f, 5.0f });
	m_renderer.CreateRenderObject("baby", m_baby.GetMeshId(), sprite_pipeline_id, m_baby.GetPipelineData());

	// ui
	AssetId arial_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetFontsPath() / "ArialAtlas.png",
		dh::PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_arial_font = std::make_unique<FontAtlas>(arial_tex_id, m_asset_manager.GetFontsPath() / "ArialAtlas.json");

	m_story_shadow.Init(m_asset_manager, 0 /*left*/, UIWidth /*right*/, UIHeight * 0.75f /*top*/, UIHeight /*bottom*/);
	m_story_shadow.SetROId(m_renderer.CreateUIRenderObject("story shadow", m_story_shadow.GetMeshId(), color_pipeline_id));

	m_fps_label.Init(m_asset_manager, *m_arial_font);
	m_renderer.CreateUIRenderObject("fps label", m_fps_label.GetUILabel()->GetMeshId(), text_pipeline_id, m_fps_label.GetUILabel()->GetPipelineData());

	m_controls_label = std::make_unique<UILabel>(m_asset_manager, "(Press [Space] to continue)", *m_arial_font,
		LabelFontSize, glm::vec2{ 960, 1026 } /*origin*/, UILabel::Align::Center, StoryTextColor);
	m_controls_label->SetROId(m_renderer.CreateUIRenderObject("controls label", m_controls_label->GetMeshId(), text_pipeline_id, m_controls_label->GetPipelineData()));

	m_story_label = std::make_unique<UILabel>(m_asset_manager, StoryTexts[0], *m_arial_font,
		LabelFontSize, glm::vec2{ 960, 918 } /*origin*/, UILabel::Align::Center, StoryTextColor);
	m_story_label->SetROId(m_renderer.CreateUIRenderObject("story label", m_story_label->GetMeshId(), text_pipeline_id, m_story_label->GetPipelineData()));

	ChangeSceneState(SceneState::Story);
}

// override
void SceneForestIntersection::OnWindowResized(int width, int height)
{
	int game_width = static_cast<int>(height * 16.0f / 9.0f);
	int x_offset = (width - game_width) / 2;
	m_renderer.SetViewport(x_offset, 0, game_width, height);
	
	m_camera3d.SetViewportSize(game_width, height);
	m_camera2d.SetViewportSize(game_width, height);
}

// override
std::optional<SceneTransition> SceneForestIntersection::Update(float dt, Input const & input)
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

	if (m_scene_state == SceneState::Gameplay && m_dog.GetPipelineData().model[3].y < -3.75)
		return SceneTransition{ SceneId::Home };

	return std::nullopt;
}

// override
void SceneForestIntersection::Render() const
{
	m_renderer.Render();
}

void SceneForestIntersection::ChangeSceneState(SceneState new_state)
{
	m_scene_state = new_state;
	
	m_grid.OnSceneStateChanged(m_scene_state, m_renderer);
	m_dog.OnSceneStateChanged(m_scene_state);
	m_baby.OnSceneStateChanged(m_scene_state);

	if (m_scene_state == SceneState::Gameplay)
	{
		m_renderer.Show(m_controls_label->GetROId(), false);
		m_renderer.Show(m_story_label->GetROId(), false);
		m_renderer.Show(m_story_shadow.GetROId(), false);
	}
}
