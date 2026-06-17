// GameplayScene.ixx

module;

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

export module GameplayScene;

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
import GameplaySceneData;
import GameplaySceneLoader;
import Input;
import IScene;
import LinePipeline;
import SceneRenderer;
import SniffTheWayConstants;
import SpritePipeline;
import StoryData;
import TextPipeline;
import UIDarkBackdrop;
import UILabel;
import Vertex;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class GameplayScene : public IScene
{
public:
	explicit GameplayScene(dh::RenderContext const & render_context, SceneId scene_id);

	void OnWindowResized(int width, int height) override;

	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void DestroyPendingAssets() const override;
	void Render() const override;

	void ChangeSceneState(SceneState new_state);

private:
	std::filesystem::path get_gameplay_filepath() const;
	void create_story_labels(PipelineId<TextPipeline> text_pipeline_id);

private:
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera3d m_camera3d;
	Camera2d m_camera2d;
	SceneState m_scene_state = SceneState::Paused;
	SceneId m_scene_id = SceneId::Exit;
	GameplaySceneData m_scene_data;

	Background m_background;
	EditorGrid m_grid;
	Dog m_dog;
	Baby m_baby;

	std::unique_ptr<FontAtlas> m_font_atlas;
	FPSLabel m_fps_label;
	UILabel m_controls_label;
	std::vector<UILabel> m_story_labels;
	UIDarkBackdrop m_story_shadow;
};

GameplayScene::GameplayScene(dh::RenderContext const & render_context, SceneId scene_id)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera3d{ render_context.ShouldFlipScreenY() }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
	, m_scene_id{ scene_id }
{
	m_scene_data = GameplaySceneLoader::LoadSceneData(get_gameplay_filepath());
	if (m_scene_data.bg_image_filename.empty())
	{
		std::cout << "GameplayScene: No data loaded for gameplay scene '" << ToString(m_scene_id) << "'." << std::endl;
		m_scene_data.bg_image_filename = "playground.png";
	}

	const auto bg_pipeline_id = m_asset_manager.AddPipeline<Texture2dPipeline>(m_camera2d, m_asset_manager);
	const auto line_pipeline_id = m_asset_manager.AddPipeline<LinePipeline>(m_camera3d);
	const auto sprite_pipeline_id = m_asset_manager.AddPipeline<SpritePipeline>(m_camera3d, m_asset_manager);
	const auto color_pipeline_id = m_asset_manager.AddPipeline<ColorPipeline>(m_camera2d);
	const auto text_pipeline_id = m_asset_manager.AddPipeline<TextPipeline>(m_camera2d, m_asset_manager);

	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);

	AssetId bg_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "gameplay_backgrounds" / m_scene_data.bg_image_filename,
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	m_background.Init(m_asset_manager, bg_tex_id);
	m_renderer.CreateRenderObject("background", RenderLayer::Background, m_background.GetMeshId(), bg_pipeline_id, m_background.GetPipelineData());

	const glm::vec3 camera_pos{ 0.0f, -6.0f, 1.0f };
	const glm::vec3 camera_dir = glm::normalize(glm::vec3{ 0.0f, 5.0f, 0.0f } - camera_pos);
	m_camera3d.Init(camera_pos, camera_dir);

	m_grid.Init(m_asset_manager);
	m_grid.SetROId(m_renderer.CreateRenderObject("grid", RenderLayer::Scene3d, m_grid.GetMeshId(), line_pipeline_id));

	m_dog.Init(m_asset_manager, camera_dir, m_scene_data.dog_spawn_pos);
	m_renderer.CreateRenderObject("dog", RenderLayer::Scene3d, m_dog.GetMeshId(), sprite_pipeline_id, m_dog.GetPipelineData());

	m_baby.Init(m_asset_manager, camera_dir, m_scene_data.baby_spawn_pos);
	m_renderer.CreateRenderObject("baby", RenderLayer::Scene3d, m_baby.GetMeshId(), sprite_pipeline_id, m_baby.GetPipelineData());

	AssetId font_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetFontsPath() / "Alice.png",
		dh::PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_font_atlas = std::make_unique<FontAtlas>(font_tex_id, m_asset_manager.GetFontsPath() / "Alice.json");

	m_story_shadow.Init(m_asset_manager, 0 /*left*/, UIWidth /*right*/, UIHeight * 0.75f /*top*/, UIHeight /*bottom*/);
	m_story_shadow.SetROId(m_renderer.CreateRenderObject("story shadow", RenderLayer::UIShadow, m_story_shadow.GetMeshId(), color_pipeline_id));

	m_fps_label.Init(m_asset_manager, m_renderer, m_camera2d, *m_font_atlas);

	m_controls_label.Init(m_asset_manager, "(Press [Space] to continue)", *m_font_atlas,
		LabelFontSize, glm::vec2{ 960, 1026 } /*origin*/, UILabel::Align::Center, StoryTextColor);
	m_controls_label.SetROId(m_renderer.CreateRenderObject("controls label", RenderLayer::UIForeground, m_controls_label.GetMeshId(), text_pipeline_id, m_controls_label.GetPipelineData()));

	create_story_labels(text_pipeline_id);

	ChangeSceneState(m_scene_data.initial_state);
}

void GameplayScene::OnWindowResized(int width, int height)
{
	int game_width = static_cast<int>(height * 16.0f / 9.0f);
	int x_offset = (width - game_width) / 2;
	m_renderer.SetViewport(x_offset, 0, game_width, height);

	m_camera3d.SetViewportSize(game_width, height);
	m_camera2d.SetViewportSize(game_width, height);
}

std::optional<SceneTransition> GameplayScene::Update(float dt, Input const & input)
{
	if (input.KeyJustPressed(Input::Key::Esc))
		return SceneTransition{ SceneId::Exit };

	if (m_scene_state == SceneState::Story && input.KeyJustPressed(Input::Key::Space))
		ChangeSceneState(SceneState::Gameplay);

	m_camera3d.Update(dt, input);
	m_fps_label.Update(dt);
	m_grid.Update(input, m_renderer, m_scene_state);
	m_dog.Update(dt, input, m_scene_data.bounds, m_scene_state);
	m_baby.Update(dt, &m_dog, m_scene_state);

	if (m_scene_state == SceneState::Gameplay)
	{
		const glm::vec2 dog_pos = m_dog.GetPipelineData().model[3];
		for (GameplayAdjacentScene const & adjacent_scene : m_scene_data.adjacent_scenes)
		{
			if (adjacent_scene.collider.Contains(dog_pos))
				return SceneTransition{ adjacent_scene.scene_id };
		}
	}

	return std::nullopt;
}

void GameplayScene::DestroyPendingAssets() const
{
	m_asset_manager.DestroyPendingAssets();
}

void GameplayScene::Render() const
{
	m_fps_label.RenderOffscreenTexture();
	m_renderer.Render();
}

void GameplayScene::ChangeSceneState(SceneState new_state)
{
	m_scene_state = new_state;

	m_grid.OnSceneStateChanged(m_scene_state, m_renderer);
	m_dog.OnSceneStateChanged(m_scene_state);
	m_baby.OnSceneStateChanged(m_scene_state);

	const bool show_story_ui = m_scene_state == SceneState::Story;
	m_renderer.Show(m_controls_label.GetROId(), show_story_ui);
	m_renderer.Show(m_story_shadow.GetROId(), show_story_ui);
	for (UILabel const & story_label : m_story_labels)
		m_renderer.Show(story_label.GetROId(), show_story_ui);
}

std::filesystem::path GameplayScene::get_gameplay_filepath() const
{
	return m_asset_manager.GetResourcesPath() / "gameplay" / (std::string{ ToString(m_scene_id) } + ".json");
}

void GameplayScene::create_story_labels(PipelineId<TextPipeline> text_pipeline_id)
{
	if (m_scene_data.story_texts.empty())
		return;
		
	m_story_labels.resize(1);
	StoryText const & story_text = m_scene_data.story_texts[0];
	UILabel & story_label = m_story_labels[0];
	story_label.Init(m_asset_manager, story_text.text, *m_font_atlas,
		story_text.font_size, story_text.pos, story_text.align, story_text.color);
	story_label.SetROId(m_renderer.CreateRenderObject("story label " + std::to_string(1),
		RenderLayer::UIForeground, story_label.GetMeshId(), text_pipeline_id, story_label.GetPipelineData()));

//	m_story_labels.resize(m_scene_data.story_texts.size());
//	for (std::size_t i = 0; i < m_scene_data.story_texts.size(); ++i)
//	{
//		StoryText const & story_text = m_scene_data.story_texts[i];
//		UILabel & story_label = m_story_labels[i];
//		story_label.Init(m_asset_manager, story_text.text, *m_font_atlas,
//			story_text.font_size, story_text.pos, story_text.align, story_text.color);
//		story_label.SetROId(m_renderer.CreateRenderObject("story label " + std::to_string(i + 1),
//			RenderLayer::UIForeground, story_label.GetMeshId(), text_pipeline_id, story_label.GetPipelineData()));
//	}
}
