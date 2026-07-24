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
import Dog;
import FontAtlas;
#ifdef _DEBUG
import FPSLabel;
#endif
import GameViewport;
#ifdef _DEBUG
import GameplaySceneEditor;
#endif
import GameplaySceneData;
import GameplaySceneLoader;
import GameplayMessageOverlay;
import Input;
import IScene;
import PauseOverlay;
import Playthrough;
import SceneRenderer;
import SceneFadeOverlay;
import SettingsOverlay;
import ScentTrail;
import ScentTrailPipeline;
import SniffTheWayConstants;
import SpritePipeline;
import TextPipeline;
import Vertex;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class GameplayScene : public IScene
{
public:
	explicit GameplayScene(
		dh::RenderContext const & render_context,
		AudioSystem & audio_system,
		Playthrough & playthrough,
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
	void order_characters_for_rendering();
	std::pair<glm::vec2, glm::vec2> get_default_spawn_positions() const;
	std::pair<glm::vec2, glm::vec2> get_spawn_positions(SceneTransition const & transition) const;
	void reset_message_triggers();
	void update_message_triggers(glm::vec2 dog_pos);
	void recreate_scent_trails(glm::vec2 dog_pos);
	void pause();
	void resume();

private:
	Playthrough & m_playthrough;
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
	AssetId m_dog_ro_id;
	AssetId m_baby_ro_id;

	FontAtlas m_font_atlas;
	PipelineId<TextPipeline> m_text_pipeline_id;
#ifdef _DEBUG
	FPSLabel m_fps_label;
#endif
	GameplayMessageOverlay m_gameplay_message_overlay;
	std::vector<bool> m_message_trigger_was_inside;
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
	Playthrough & playthrough,
	SceneId scene_id,
	SceneTransition const & transition)
	: m_playthrough{ playthrough }
	, m_asset_manager{ render_context }
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
	m_dog_ro_id = m_renderer.CreateRenderObject(
		"dog", RenderLayer::Scene3d, m_dog.GetMeshId(), sprite_pipeline_id, m_dog.GetPipelineData());

	m_baby.Init(m_asset_manager, m_camera3d.GetDir(), baby_spawn_pos);
	m_baby_ro_id = m_renderer.CreateRenderObject(
		"baby", RenderLayer::Scene3d, m_baby.GetMeshId(), sprite_pipeline_id, m_baby.GetPipelineData());
	order_characters_for_rendering();

#ifdef _DEBUG
	m_fps_label.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas);
#endif

	m_gameplay_message_overlay.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas);
	reset_message_triggers();
	m_pause_overlay.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas, audio_system);
	m_settings_overlay.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas, audio_system);
	m_scene_fade_overlay.Init(m_asset_manager, m_renderer, m_camera2d);

	ChangeSceneState(SceneState::Gameplay);
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
			return SceneTransition{ SceneId::Title, m_scene_id, PlaythroughAction::End };
		case PauseAction::Exit:
			return SceneTransition{ SceneId::Exit };
		case PauseAction::None:
			break;
		}
		return std::nullopt;
	}

	m_camera3d.Update(dt, input);

#ifdef _DEBUG
	m_fps_label.Update(dt);
#endif

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
	order_characters_for_rendering();

	const glm::vec2 dog_pos = m_dog.GetPosition();
	for (std::size_t i = 0; i < m_scene_data.scent_trails.size() && i < m_scent_trails.size(); ++i)
		m_scent_trails[i].Update(dt, dog_pos);

	if (m_scene_state == SceneState::Gameplay)
	{
		m_gameplay_message_overlay.Update(dt);
		update_message_triggers(dog_pos);

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
#ifdef _DEBUG
	m_fps_label.RenderOffscreenTexture();
#endif
	m_gameplay_message_overlay.RenderOffscreenTexture();
	m_renderer.Render();
}

void GameplayScene::ChangeSceneState(SceneState new_state)
{
	m_scene_state = new_state;

	m_dog.OnSceneStateChanged(m_scene_state);
	m_baby.OnSceneStateChanged(m_scene_state);

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
	m_gameplay_message_overlay.Hide();
	reset_message_triggers();
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

void GameplayScene::order_characters_for_rendering()
{
	// Draw the farther transparent sprite first so the nearer sprite's soft edges blend over it.
	glm::vec3 const camera_pos = m_camera3d.GetPosition();
	glm::vec3 const camera_dir = m_camera3d.GetDir();
	float const dog_depth = glm::dot(glm::vec3{ m_dog.GetPosition(), 0.0f } - camera_pos, camera_dir);
	float const baby_depth = glm::dot(glm::vec3{ m_baby.GetPosition(), 0.0f } - camera_pos, camera_dir);

	constexpr float DepthEpsilon = 1e-4f;
	if (dog_depth > baby_depth + DepthEpsilon)
		m_renderer.SetRenderObjectBefore(RenderLayer::Scene3d, m_dog_ro_id, m_baby_ro_id);
	else if (baby_depth > dog_depth + DepthEpsilon)
		m_renderer.SetRenderObjectBefore(RenderLayer::Scene3d, m_baby_ro_id, m_dog_ro_id);
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

void GameplayScene::reset_message_triggers()
{
	m_message_trigger_was_inside.assign(m_scene_data.message_triggers.size(), false);
}

void GameplayScene::update_message_triggers(glm::vec2 dog_pos)
{
	for (std::size_t i = 0; i < m_scene_data.message_triggers.size(); ++i)
	{
		GameplayMessageTriggerData const & message_trigger = m_scene_data.message_triggers[i];
		const bool is_inside = message_trigger.trigger.IsValid()
			&& message_trigger.trigger.Contains(dog_pos);
		const bool entered = is_inside && !m_message_trigger_was_inside[i];
		m_message_trigger_was_inside[i] = is_inside;

		if (!entered || message_trigger.id.empty())
			continue;

		switch (message_trigger.repeat)
		{
		case GameplayMessageRepeat::None:
			if (m_playthrough.TryTrigger(m_scene_id, message_trigger.id))
				m_gameplay_message_overlay.Show(message_trigger.message);
			break;
		}
	}
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
