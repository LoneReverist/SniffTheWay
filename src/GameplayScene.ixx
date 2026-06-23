// GameplayScene.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
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
import FontAtlas;
import FPSLabel;
#ifdef _DEBUG
import GameplaySceneEditor;
#endif
import GameplaySceneData;
import GameplaySceneLoader;
import Input;
import IScene;
import RenderObject;
import SceneRenderer;
import ScentTrail;
import ScentTrailPipeline;
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
	explicit GameplayScene(dh::RenderContext const & render_context, SceneId scene_id, SceneTransition const & transition);

	void OnWindowResized(int width, int height) override;

	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void DestroyPendingAssets() const override;
	void Render() const override;

	void ChangeSceneState(SceneState new_state);

private:
	std::filesystem::path get_gameplay_filepath() const;
	void reload_scene_data();
	void reload_background_texture();
	void apply_story_labels();
	std::pair<glm::vec2, glm::vec2> get_spawn_positions(SceneTransition const & transition) const;
	void create_story_labels(PipelineId<TextPipeline> text_pipeline_id);
	void create_or_reload_scent_trail(glm::vec2 dog_pos);

private:
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera3d m_camera3d;
	Camera2d m_camera2d;
	glm::ivec4 m_game_viewport{ 0, 0, 0, 0 };
	SceneState m_scene_state = SceneState::Paused;
	SceneId m_scene_id = SceneId::Exit;
	GameplaySceneData m_scene_data;

	Background m_background;
	AssetId m_bg_tex_id;
	ScentTrail m_scent_trail;
	PipelineId<ScentTrailPipeline> m_scent_trail_pipeline_id;
	AssetId m_scent_trail_ro_id;
	Dog m_dog;
	Baby m_baby;

	FontAtlas m_font_atlas;
	PipelineId<TextPipeline> m_text_pipeline_id;
	FPSLabel m_fps_label;
	UILabel m_controls_label;
	std::vector<UILabel> m_story_labels;
	UIDarkBackdrop m_story_shadow;

#ifdef _DEBUG
	GameplaySceneEditor m_editor;
#endif
};

GameplayScene::GameplayScene(dh::RenderContext const & render_context, SceneId scene_id, SceneTransition const & transition)
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

	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);

	const glm::vec3 camera_pos{ 0.0f, -6.0f, 1.0f };
	const glm::vec3 camera_dir = glm::normalize(glm::vec3{ 0.0f, 5.0f, 0.0f } - camera_pos);
	m_camera3d.Init(camera_pos, camera_dir);

	AssetId font_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetFontsPath() / "Alice.png",
		dh::PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_font_atlas.Init(font_tex_id, m_asset_manager.GetFontsPath() / "Alice.json");

	const auto bg_pipeline_id = m_asset_manager.AddPipeline<Texture2dPipeline>(m_camera2d, m_asset_manager);
	const auto sprite_pipeline_id = m_asset_manager.AddPipeline<SpritePipeline>(m_camera3d, m_asset_manager);
	m_scent_trail_pipeline_id = m_asset_manager.AddPipeline<ScentTrailPipeline>(m_camera3d);
	const auto color_pipeline_id = m_asset_manager.AddPipeline<ColorPipeline>(m_camera2d);
	m_text_pipeline_id = m_asset_manager.AddPipeline<TextPipeline>(m_camera2d, m_asset_manager);

	m_bg_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "gameplay_backgrounds" / m_scene_data.bg_image_filename,
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	m_background.Init(m_asset_manager, m_bg_tex_id);
	m_renderer.CreateRenderObject("background", RenderLayer::Background, m_background.GetMeshId(), bg_pipeline_id, m_background.GetPipelineData());

	const auto [dog_spawn_pos, baby_spawn_pos] = get_spawn_positions(transition);
	create_or_reload_scent_trail(dog_spawn_pos);

#ifdef _DEBUG
	m_editor.Init(m_asset_manager, m_renderer, m_camera3d, m_font_atlas, m_text_pipeline_id, m_scene_data, get_gameplay_filepath());
#endif

	m_dog.Init(m_asset_manager, camera_dir, dog_spawn_pos);
	m_renderer.CreateRenderObject("dog", RenderLayer::Scene3d, m_dog.GetMeshId(), sprite_pipeline_id, m_dog.GetPipelineData());

	m_baby.Init(m_asset_manager, camera_dir, baby_spawn_pos);
	m_renderer.CreateRenderObject("baby", RenderLayer::Scene3d, m_baby.GetMeshId(), sprite_pipeline_id, m_baby.GetPipelineData());

	m_story_shadow.Init(m_asset_manager, 0 /*left*/, UIWidth /*right*/, UIHeight * 0.75f /*top*/, UIHeight /*bottom*/);
	m_story_shadow.SetROId(m_renderer.CreateRenderObject("story shadow", RenderLayer::UIShadow, m_story_shadow.GetMeshId(), color_pipeline_id));

	m_fps_label.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas);

	m_controls_label.Init(m_asset_manager, "(Press [Space] to continue)", m_font_atlas,
		LabelFontSize, glm::vec2{ 960, 1026 } /*origin*/, UILabel::Align::Center, StoryTextColor);
	m_controls_label.SetROId(m_renderer.CreateRenderObject("controls label", RenderLayer::UIForeground, m_controls_label.GetMeshId(), m_text_pipeline_id, m_controls_label.GetPipelineData()));

	create_story_labels(m_text_pipeline_id);

	const bool arrived_from_gameplay_scene = transition.previous_scene_id.has_value()
		&& IsGameplayScene(transition.previous_scene_id.value());
	ChangeSceneState(arrived_from_gameplay_scene ? SceneState::Gameplay : m_scene_data.initial_state);
}

void GameplayScene::OnWindowResized(int width, int height)
{
	int game_width = static_cast<int>(height * 16.0f / 9.0f);
	int x_offset = (width - game_width) / 2;
	m_game_viewport = glm::ivec4{ x_offset, 0, game_width, height };
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

#ifdef _DEBUG
	if ((m_scene_state == SceneState::Gameplay || m_scene_state == SceneState::Editing) && input.KeyJustPressed('E'))
	{
		ChangeSceneState(m_scene_state == SceneState::Editing ? SceneState::Gameplay : SceneState::Editing);
		return std::nullopt;
	}

	m_editor.Update(input, m_asset_manager, m_renderer, m_camera3d, m_game_viewport, m_scene_state);
	if (m_editor.ConsumeScentTrailChanged())
		create_or_reload_scent_trail(m_dog.GetPosition());

	if (input.KeyJustPressed('R'))
		reload_scene_data();
#endif

	m_dog.Update(dt, input, m_scene_data.bounds, m_scene_state);
	m_baby.Update(dt, &m_dog, m_scene_state);

	const glm::vec2 dog_pos = m_dog.GetPosition();
	m_scent_trail.Update(dt, dog_pos);

	if (m_scene_state == SceneState::Gameplay)
	{
		for (GameplayAdjacentScene const & adjacent_scene : m_scene_data.adjacent_scenes)
		{
			if (adjacent_scene.collider.Contains(dog_pos))
				return SceneTransition{ adjacent_scene.scene_id, m_scene_id };
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

	m_dog.OnSceneStateChanged(m_scene_state);
	m_baby.OnSceneStateChanged(m_scene_state);

	const bool show_story_ui = m_scene_state == SceneState::Story;
	m_renderer.Show(m_controls_label.GetROId(), show_story_ui);
	m_renderer.Show(m_story_shadow.GetROId(), show_story_ui);
	for (UILabel const & story_label : m_story_labels)
		m_renderer.Show(story_label.GetROId(), show_story_ui);

#ifdef _DEBUG
	m_editor.OnSceneStateChanged(m_scene_state, m_asset_manager, m_renderer);
	const float character_opacity = m_scene_state == SceneState::Editing ? 0.3f : 1.0f;
	m_dog.SetOpacity(character_opacity);
	m_baby.SetOpacity(character_opacity);
#endif
}

std::filesystem::path GameplayScene::get_gameplay_filepath() const
{
	return m_asset_manager.GetResourcesPath() / "gameplay" / (std::string{ ToString(m_scene_id) } + ".json");
}

void GameplayScene::reload_scene_data()
{
	GameplaySceneData reloaded_scene_data = GameplaySceneLoader::LoadSceneData(get_gameplay_filepath());
	if (reloaded_scene_data.bg_image_filename.empty())
	{
		std::cout << "GameplayScene: Reload skipped because no data loaded for gameplay scene '" << ToString(m_scene_id) << "'." << std::endl;
		return;
	}

	m_scene_data = std::move(reloaded_scene_data);
	reload_background_texture();
	apply_story_labels();
	m_dog.Reload(m_camera3d.GetDir(), m_scene_data.dog_spawn_pos);
	m_baby.Reload(m_camera3d.GetDir(), m_scene_data.baby_spawn_pos);
	create_or_reload_scent_trail(m_scene_data.dog_spawn_pos);

#ifdef _DEBUG
	m_editor.Reload(m_asset_manager, m_renderer);
#endif

	if (m_scene_state == SceneState::Editing)
		ChangeSceneState(SceneState::Gameplay);
	else
		ChangeSceneState(m_scene_state);
	
	std::cout << "GameplayScene: Reloaded gameplay scene '" << ToString(m_scene_id) << "'." << std::endl;
}

void GameplayScene::reload_background_texture()
{
	AssetId old_bg_tex_id = m_bg_tex_id;
	AssetId new_bg_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "gameplay_backgrounds" / m_scene_data.bg_image_filename,
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	if (!new_bg_tex_id.IsValid())
	{
		std::cout << "GameplayScene: Keeping previous background because reload failed for '" << m_scene_data.bg_image_filename << "'." << std::endl;
		return;
	}

	m_bg_tex_id = new_bg_tex_id;
	m_background.SetTextureId(m_bg_tex_id);

	if (old_bg_tex_id.IsValid())
		m_asset_manager.RemoveTexture(old_bg_tex_id);
}

void GameplayScene::apply_story_labels()
{
	if (m_story_labels.empty())
	{
		if (!m_scene_data.story_texts.empty())
			create_story_labels(m_text_pipeline_id);
		return;
	}

	if (m_scene_data.story_texts.empty())
	{
		m_renderer.Show(m_story_labels[0].GetROId(), false);
		return;
	}

	StoryText const & story_text = m_scene_data.story_texts[0];
	UILabel & story_label = m_story_labels[0];
	story_label.SetText(story_text.text);
	story_label.SetFontSize(story_text.font_size);
	story_label.SetOrigin(story_text.pos);
	story_label.SetAlign(story_text.align);
	story_label.SetTextColor(story_text.color);
	m_renderer.Show(story_label.GetROId(), m_scene_state == SceneState::Story);
}

std::pair<glm::vec2, glm::vec2> GameplayScene::get_spawn_positions(SceneTransition const & transition) const
{
	if (transition.previous_scene_id.has_value())
	{
		for (GameplayAdjacentScene const & adjacent_scene : m_scene_data.adjacent_scenes)
		{
			if (adjacent_scene.scene_id == transition.previous_scene_id.value())
				return { adjacent_scene.dog_entry_spawn_pos, adjacent_scene.baby_entry_spawn_pos };
		}
	}

	return { m_scene_data.dog_spawn_pos, m_scene_data.baby_spawn_pos };
}

void GameplayScene::create_story_labels(PipelineId<TextPipeline> text_pipeline_id)
{
	if (m_scene_data.story_texts.empty())
		return;
		
	m_story_labels.resize(1);
	StoryText const & story_text = m_scene_data.story_texts[0];
	UILabel & story_label = m_story_labels[0];
	story_label.Init(m_asset_manager, story_text.text, m_font_atlas,
		story_text.font_size, story_text.pos, story_text.align, story_text.color);
	story_label.SetROId(m_renderer.CreateRenderObject("story label " + std::to_string(1),
		RenderLayer::UIForeground, story_label.GetMeshId(), text_pipeline_id, story_label.GetPipelineData()));

//	m_story_labels.resize(m_scene_data.story_texts.size());
//	for (std::size_t i = 0; i < m_scene_data.story_texts.size(); ++i)
//	{
//		StoryText const & story_text = m_scene_data.story_texts[i];
//		UILabel & story_label = m_story_labels[i];
//		story_label.Init(m_asset_manager, story_text.text, m_font_atlas,
//			story_text.font_size, story_text.pos, story_text.align, story_text.color);
//		story_label.SetROId(m_renderer.CreateRenderObject("story label " + std::to_string(i + 1),
//			RenderLayer::UIForeground, story_label.GetMeshId(), text_pipeline_id, story_label.GetPipelineData()));
//	}
}

void GameplayScene::create_or_reload_scent_trail(glm::vec2 dog_pos)
{
	if (m_scent_trail_ro_id.IsValid())
	{
		m_scent_trail.Reload(m_asset_manager, m_scene_data.scent_trail, dog_pos);

		RenderObject * scent_trail_ro = m_renderer.GetRenderObject(m_scent_trail_ro_id);
		if (scent_trail_ro && m_scent_trail.IsValid())
			scent_trail_ro->SetMeshId(m_scent_trail.GetMeshId());

		m_renderer.Show(m_scent_trail_ro_id, m_scent_trail.IsValid());
		return;
	}

	m_scent_trail.Init(m_asset_manager, m_scene_data.scent_trail, dog_pos);
	if (!m_scent_trail.IsValid())
		return;

	m_scent_trail_ro_id = m_renderer.CreateRenderObject("scent trail",
		RenderLayer::Scene3d, m_scent_trail.GetMeshId(), m_scent_trail_pipeline_id, m_scent_trail.GetPipelineData());
}
