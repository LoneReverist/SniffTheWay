// GameplayScene.ixx

module;

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <glm/glm.hpp>
#include <glog/logging.h>

export module GameplayScene;

import Dreamhearth;

import AssetManager;
import AssetPool;
import AudioSystem;
import Baby;
import Background;
import Texture2dPipeline;
import Camera;
import ColorPipeline;
import Dog;
import FontAtlas;
import FPSLabel;
import GameViewport;
#ifdef _DEBUG
import GameplaySceneEditor;
#endif
import GameplaySceneData;
import GameplaySceneLoader;
import Input;
import IScene;
import PauseOverlay;
import SceneRenderer;
import SceneFadeOverlay;
import SettingsOverlay;
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
	explicit GameplayScene(
		dh::RenderContext const & render_context,
		AudioSystem & audio_system,
		SceneId scene_id,
		SceneTransition const & transition);

	void OnViewportChanged(GameViewport const & viewport) override;

	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void DestroyPendingAssets() const override;
	void SetTransitionOpacity(float opacity) override;
	void Render() const override;

	void ChangeSceneState(SceneState new_state);

private:
	std::filesystem::path get_gameplay_filepath() const;
	void reload_scene_data();
	void reload_background_texture();
	void apply_camera_data();
	void store_camera_data();
	void refresh_character_camera_facing();
	void apply_story_labels();
	std::pair<glm::vec2, glm::vec2> get_default_spawn_positions() const;
	std::pair<glm::vec2, glm::vec2> get_spawn_positions(SceneTransition const & transition) const;
	void create_story_labels(PipelineId<TextPipeline> text_pipeline_id);
	void recreate_scent_trails(glm::vec2 dog_pos);
	void pause();
	void resume();

private:
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera3d m_camera3d;
	Camera2d m_camera2d;
	GameViewport m_game_viewport;
	SceneState m_scene_state = SceneState::Paused;
	SceneState m_state_before_pause = SceneState::Gameplay;
	SceneId m_scene_id = SceneId::Exit;
	GameplaySceneData m_scene_data;

	Background m_background;
	AssetId m_bg_tex_id;
	std::vector<ScentTrail> m_scent_trails;
	PipelineId<ScentTrailPipeline> m_scent_trail_pipeline_id;
	std::vector<AssetId> m_scent_trail_ro_ids;
	Dog m_dog;
	Baby m_baby;

	FontAtlas m_font_atlas;
	PipelineId<TextPipeline> m_text_pipeline_id;
	FPSLabel m_fps_label;
	UILabel m_controls_label;
	std::vector<UILabel> m_story_labels;
	UIDarkBackdrop m_story_shadow;
	PauseOverlay m_pause_overlay;
	SettingsOverlay m_settings_overlay;
	SceneFadeOverlay m_scene_fade_overlay;

#ifdef _DEBUG
	GameplaySceneEditor m_editor;
#endif
};

GameplayScene::GameplayScene(
	dh::RenderContext const & render_context,
	AudioSystem & audio_system,
	SceneId scene_id,
	SceneTransition const & transition)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera3d{ render_context.ShouldFlipScreenY() }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
	, m_scene_id{ scene_id }
{
	m_scene_data = GameplaySceneLoader::LoadSceneData(get_gameplay_filepath());
	if (m_scene_data.bg_image_filename.empty())
	{
		LOG(WARNING) << "GameplayScene: No data loaded for gameplay scene '" << ToString(m_scene_id) << "'.";
		m_scene_data.bg_image_filename = "playground.png";
	}

	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);
	apply_camera_data();

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
	recreate_scent_trails(dog_spawn_pos);

#ifdef _DEBUG
	m_editor.Init(m_asset_manager, m_renderer, m_camera3d, m_font_atlas, m_text_pipeline_id, m_scene_data, get_gameplay_filepath());
#endif

	m_dog.Init(m_asset_manager, m_camera3d.GetDir(), dog_spawn_pos);
	m_renderer.CreateRenderObject("dog", RenderLayer::Scene3d, m_dog.GetMeshId(), sprite_pipeline_id, m_dog.GetPipelineData());

	m_baby.Init(m_asset_manager, m_camera3d.GetDir(), baby_spawn_pos);
	m_renderer.CreateRenderObject("baby", RenderLayer::Scene3d, m_baby.GetMeshId(), sprite_pipeline_id, m_baby.GetPipelineData());

	m_story_shadow.Init(m_asset_manager, 0 /*left*/, UIWidth /*right*/, UIHeight * 0.75f /*top*/, UIHeight /*bottom*/, 0.6f /*alpha_top*/, 1.0f /*alpha_bottom*/);
	m_story_shadow.SetROId(m_renderer.CreateRenderObject("story shadow", RenderLayer::UIShadow, m_story_shadow.GetMeshId(), color_pipeline_id));

	m_fps_label.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas);

	m_controls_label.Init(m_asset_manager, "(Press [Space] to continue)", m_font_atlas,
		LabelFontSize, glm::vec2{ 960, 1026 } /*origin*/, UILabel::Align::Center, StoryTextColor);
	m_controls_label.SetROId(m_renderer.CreateRenderObject("controls label", RenderLayer::UIForeground, m_controls_label.GetMeshId(), m_text_pipeline_id, m_controls_label.GetPipelineData()));

	create_story_labels(m_text_pipeline_id);
	m_pause_overlay.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas, audio_system);
	m_settings_overlay.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas, audio_system);
	m_scene_fade_overlay.Init(m_asset_manager, m_renderer, m_camera2d);

	const bool arrived_from_gameplay_scene = transition.previous_scene_id.has_value()
		&& IsGameplayScene(transition.previous_scene_id.value());
	ChangeSceneState(arrived_from_gameplay_scene ? SceneState::Gameplay : m_scene_data.initial_state);
}

void GameplayScene::OnViewportChanged(GameViewport const & viewport)
{
	m_game_viewport = viewport;
	const auto & pixels = viewport.pixels;
	m_renderer.SetViewport(pixels.x, pixels.y, pixels.z, pixels.w);
	m_camera3d.SetViewportSize(pixels.z, pixels.w);
	m_camera2d.SetViewportSize(pixels.z, pixels.w);
}

std::optional<SceneTransition> GameplayScene::Update(float dt, Input const & input)
{
	if (m_settings_overlay.IsVisible())
	{
		if (m_settings_overlay.Update(input, m_game_viewport))
		{
			m_settings_overlay.SetVisible(false);
			m_pause_overlay.SetVisible(true);
		}
		return std::nullopt;
	}

	if (m_scene_state != SceneState::Editing && input.KeyJustPressed(Input::Key::Esc))
	{
		if (m_scene_state == SceneState::Paused)
			resume();
		else
			pause();
		return std::nullopt;
	}

	if (m_scene_state == SceneState::Paused)
	{
		switch (m_pause_overlay.Update(input, m_game_viewport))
		{
		case PauseAction::Resume:
			resume();
			break;
		case PauseAction::Settings:
			m_pause_overlay.SetVisible(false);
			m_settings_overlay.SetVisible(true);
			break;
		case PauseAction::ReturnToTitle:
			return SceneTransition{ SceneId::Title, m_scene_id };
		case PauseAction::Exit:
			return SceneTransition{ SceneId::Exit };
		case PauseAction::None:
			break;
		}
		return std::nullopt;
	}

	if (m_scene_state == SceneState::Story && input.KeyJustPressed(Input::Key::Space))
		ChangeSceneState(SceneState::Gameplay);

	m_camera3d.Update(dt, input);
	m_fps_label.Update(dt);

#ifdef _DEBUG
	if (m_scene_state == SceneState::Gameplay)
	{
		for (int scene_link_number = 1; scene_link_number <= 4; ++scene_link_number)
		{
			if (!input.KeyJustPressed('0' + scene_link_number))
				continue;

			const int scene_link_index = scene_link_number - 1;
			if (scene_link_index < static_cast<int>(m_scene_data.scene_links.size()))
			{
				GameplaySceneLink const & scene_link = m_scene_data.scene_links[scene_link_index];
				return SceneTransition{ scene_link.target_scene_id, m_scene_id };
			}
		}
	}

	if (m_scene_state == SceneState::Gameplay && input.KeyJustPressed('E'))
	{
		ChangeSceneState(SceneState::Editing);
		return std::nullopt;
	}
	if (m_scene_state == SceneState::Editing
		&& input.KeyJustPressed(Input::Key::Esc)
		&& !m_editor.HasActiveEditMode())
	{
		ChangeSceneState(SceneState::Gameplay);
		return std::nullopt;
	}

	if (!m_editor.HasActiveEditMode() && input.KeyJustPressed('R'))
		reload_scene_data();

	m_editor.Update(input, m_asset_manager, m_renderer, m_camera3d, m_game_viewport.pixels, m_scene_state);
	if (m_editor.ConsumeCameraChanged())
	{
		store_camera_data();
		refresh_character_camera_facing();
	}
	if (m_editor.ConsumeScentTrailChanged())
		recreate_scent_trails(m_dog.GetPosition());
#endif

	m_dog.Update(dt, input, m_scene_data.bounds, m_scene_state);
	m_baby.Update(dt, &m_dog, m_scene_state);

	const glm::vec2 dog_pos = m_dog.GetPosition();
	for (std::size_t i = 0; i < m_scene_data.scent_trails.size() && i < m_scent_trails.size(); ++i)
		m_scent_trails[i].Update(dt, dog_pos);

	if (m_scene_state == SceneState::Gameplay)
	{
		for (GameplaySceneLink const & scene_link : m_scene_data.scene_links)
		{
			if (scene_link.trigger.Contains(dog_pos))
				return SceneTransition{ scene_link.target_scene_id, m_scene_id };
		}
	}

	return std::nullopt;
}

void GameplayScene::DestroyPendingAssets() const
{
	m_asset_manager.DestroyPendingAssets();
}

void GameplayScene::SetTransitionOpacity(float opacity)
{
	m_scene_fade_overlay.SetOpacity(opacity);
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

	const SceneState visible_scene_state = m_scene_state == SceneState::Paused
		? m_state_before_pause
		: m_scene_state;
	const bool show_story_ui = visible_scene_state == SceneState::Story;
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

void GameplayScene::pause()
{
	m_state_before_pause = m_scene_state;
	ChangeSceneState(SceneState::Paused);
	m_pause_overlay.SetVisible(true);
	m_settings_overlay.SetVisible(false);
}

void GameplayScene::resume()
{
	m_pause_overlay.SetVisible(false);
	m_settings_overlay.SetVisible(false);
	ChangeSceneState(m_state_before_pause);
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
		LOG(WARNING) << "GameplayScene: Reload skipped because no data loaded for gameplay scene '" << ToString(m_scene_id) << "'.";
		return;
	}

	m_scene_data = std::move(reloaded_scene_data);
	apply_camera_data();
	reload_background_texture();
	apply_story_labels();
	const auto [dog_spawn_pos, baby_spawn_pos] = get_default_spawn_positions();
	m_dog.Reload(m_camera3d.GetDir(), dog_spawn_pos);
	m_baby.Reload(m_camera3d.GetDir(), baby_spawn_pos);
	recreate_scent_trails(dog_spawn_pos);

#ifdef _DEBUG
	m_editor.Reload(m_asset_manager, m_renderer);
#endif

	if (m_scene_state == SceneState::Editing)
		ChangeSceneState(SceneState::Gameplay);
	else
		ChangeSceneState(m_scene_state);
	
	LOG(INFO) << "GameplayScene: Reloaded gameplay scene '" << ToString(m_scene_id) << "'.";
}

void GameplayScene::apply_camera_data()
{
	m_camera3d.Init(m_scene_data.camera.position, m_scene_data.camera.direction, m_scene_data.camera.fov_degrees);
}

void GameplayScene::store_camera_data()
{
	m_scene_data.camera.position = m_camera3d.GetPosition();
	m_scene_data.camera.direction = m_camera3d.GetDir();
	m_scene_data.camera.fov_degrees = m_camera3d.GetFovDegrees();
}

void GameplayScene::refresh_character_camera_facing()
{
	m_dog.Reload(m_camera3d.GetDir(), m_dog.GetPosition());
	m_baby.Reload(m_camera3d.GetDir(), m_baby.GetPosition());
}

void GameplayScene::reload_background_texture()
{
	AssetId old_bg_tex_id = m_bg_tex_id;
	AssetId new_bg_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "gameplay_backgrounds" / m_scene_data.bg_image_filename,
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	if (!new_bg_tex_id.IsValid())
	{
		LOG(WARNING) << "GameplayScene: Keeping previous background because reload failed for '"
			<< m_scene_data.bg_image_filename << "'.";
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
		for (GameplaySceneLink const & scene_link : m_scene_data.scene_links)
		{
			if (scene_link.target_scene_id == transition.previous_scene_id.value())
				return { scene_link.dog_arrival_pos, scene_link.baby_arrival_pos };
		}
	}

	return get_default_spawn_positions();
}

std::pair<glm::vec2, glm::vec2> GameplayScene::get_default_spawn_positions() const
{
	if (!m_scene_data.scene_links.empty())
	{
		GameplaySceneLink const & default_spawn_link = m_scene_data.scene_links.front();
		return { default_spawn_link.dog_arrival_pos, default_spawn_link.baby_arrival_pos };
	}

	return { glm::vec2{ 0.0f }, glm::vec2{ 0.0f } };
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

void GameplayScene::recreate_scent_trails(glm::vec2 dog_pos)
{
	for (AssetId scent_trail_ro_id : m_scent_trail_ro_ids)
		m_renderer.RemoveRenderObject(scent_trail_ro_id);
	for (ScentTrail & scent_trail : m_scent_trails)
		scent_trail.Destroy(m_asset_manager);

	m_scent_trail_ro_ids.clear();
	m_scent_trails.clear();
	m_scent_trails.resize(m_scene_data.scent_trails.size());
	m_scent_trail_ro_ids.reserve(m_scene_data.scent_trails.size());

	for (std::size_t i = 0; i < m_scene_data.scent_trails.size(); ++i)
	{
		ScentTrail & scent_trail = m_scent_trails[i];
		scent_trail.Init(m_asset_manager, m_scene_data.scent_trails[i], dog_pos);
		m_scent_trail_ro_ids.push_back(scent_trail.IsValid()
			? m_renderer.CreateRenderObject("scent trail " + std::to_string(i + 1),
				RenderLayer::Scene3d, scent_trail.GetMeshId(), m_scent_trail_pipeline_id, scent_trail.GetPipelineData())
			: AssetId{});
	}
}
