// StoryScene.ixx

module;

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <glm/glm.hpp>
#include <glog/logging.h>

export module StoryScene;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Background;
import Texture2dPipeline;
import Camera;
import DecorationAtlas;
import FontAtlas;
import FPSLabel;
import GameViewport;
import Input;
import IScene;
import PauseOverlay;
import SceneRenderer;
import SceneFadeOverlay;
import SniffTheWayConstants;
import StoryData;
import StoryLoader;
import UILabel;
import UIShadowedDecoration;
import UIShadowedLabel;
import Vertex;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class StoryScene : public IScene
{
public:
	explicit StoryScene(
		dh::RenderContext const & render_context,
		SceneId scene_id);

	void OnViewportChanged(GameViewport const & viewport) override;

	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void DestroyPendingAssets() const override;
	void SetTransitionOpacity(float opacity) override;
	void Render() const override;

	void ChangeSceneState(SceneState new_state);

private:
	enum class PageTransitionState
	{
		None,
		FadingOut,
		FadingIn,
	};

	bool page_forward();
	void page_backward();
	void start_page_transition(std::uint8_t page_index);
	void update_page_transition(float dt);
	std::filesystem::path get_story_filepath() const;
	void load_background_textures();
	void ensure_story_label_count(std::size_t count);
	void ensure_decoration_count(std::size_t count);
	void ensure_story_ui_capacity();
	void reload_story();
	void apply_current_page();
	void update_story_texts();
	void update_decorations();
	void update_controls_label();
	float get_current_page_reveal_end_time() const;
	void pause();
	void resume();

private:
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera2d m_camera2d;
	GameViewport m_viewport;
	SceneState m_scene_state = SceneState::Paused;
	SceneState m_state_before_pause = SceneState::Story;
	SceneId m_scene_id = SceneId::Exit;
	StorySceneData m_scene_data;
	std::uint8_t m_page_index = 0;
	float m_page_time = 0.0f;
	std::uint8_t m_pending_page_index = 0;
	PageTransitionState m_page_transition_state = PageTransitionState::None;
	float m_page_transition_opacity = 0.0f;
	static constexpr float PageTransitionDuration = 0.5f;

	Background m_background;
	std::vector<AssetId> m_bg_tex_ids;

	FontAtlas m_font_atlas;
	AssetId m_decoration_tex_id;
	FPSLabel m_fps_label;
	UIShadowedLabel m_controls_label;
	UIShadowedLabel m_page_number_label;
	std::vector<UIShadowedLabel> m_story_labels;
	std::vector<UIShadowedDecoration> m_decorations;
	PauseOverlay m_pause_overlay;
	SceneFadeOverlay m_page_fade_overlay;
	SceneFadeOverlay m_scene_fade_overlay;
};

StoryScene::StoryScene(
	dh::RenderContext const & render_context,
	SceneId scene_id)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
	, m_scene_id{ scene_id }
{
	m_scene_data = StoryLoader::LoadSceneData(get_story_filepath());
	if (m_scene_data.pages.empty())
	{
		LOG(WARNING) << "StoryScene: No pages loaded for story '" << ToString(m_scene_id) << "'.";
		m_scene_data.pages.push_back(StoryPage{ .bg_image_filename = "picnic.png" });
	}

	const auto bg_pipeline_id = m_asset_manager.AddPipeline<Texture2dPipeline>(m_camera2d, m_asset_manager);

	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);

	load_background_textures();
	m_page_index = 0;
	m_background.Init(m_asset_manager, m_bg_tex_ids[m_page_index]);
	m_renderer.CreateRenderObject("background", RenderLayer::Background, m_background.GetMeshId(), bg_pipeline_id, m_background.GetPipelineData());

	m_decoration_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / Decorations::TextureFileName,
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	AssetId font_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetFontsPath() / "Alice.png",
		dh::PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_font_atlas.Init(font_tex_id, m_asset_manager.GetFontsPath() / "Alice.json");

	m_fps_label.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas);

	m_controls_label.Init(
		m_asset_manager,
		m_renderer,
		m_camera2d,
		"controls",
		"(Press [Space] to continue)",
		m_font_atlas,
		LabelFontSize,
		glm::vec2{ 960, 1026 } /*origin*/,
		UILabel::Align::Center,
		StoryTextColor);

	std::string page_number_text = std::to_string(m_page_index + 1) + "/" + std::to_string(m_bg_tex_ids.size());
	m_page_number_label.Init(
		m_asset_manager,
		m_renderer,
		m_camera2d,
		"page number",
		page_number_text,
		m_font_atlas,
		LabelFontSize,
		glm::vec2{ 1824, 1026 } /*origin*/,
		UILabel::Align::Right,
		StoryTextColor);

	ensure_story_ui_capacity();

	apply_current_page();
	m_pause_overlay.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas);
	m_page_fade_overlay.Init(m_asset_manager, m_renderer, m_camera2d);
	m_page_fade_overlay.SetOpacity(0.0f);
	m_scene_fade_overlay.Init(m_asset_manager, m_renderer, m_camera2d);

	ChangeSceneState(SceneState::Story);
}

void StoryScene::OnViewportChanged(GameViewport const & viewport)
{
	m_viewport = viewport;
	const auto & pixels = viewport.pixels;
	m_renderer.SetViewport(pixels.x, pixels.y, pixels.z, pixels.w);
	m_camera2d.SetViewportSize(pixels.z, pixels.w);
}

std::optional<SceneTransition> StoryScene::Update(float dt, Input const & input)
{
	if (m_page_transition_state != PageTransitionState::None)
	{
		update_page_transition(dt);
		m_fps_label.Update(dt);
		return std::nullopt;
	}

	if (input.KeyJustPressed(Input::Key::Esc))
	{
		if (m_scene_state == SceneState::Paused)
			resume();
		else
			pause();
		return std::nullopt;
	}

	if (m_scene_state == SceneState::Paused)
	{
		switch (m_pause_overlay.Update(input, m_viewport))
		{
		case PauseAction::Resume:
			resume();
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

	if (m_scene_state == SceneState::Story)
	{
		m_page_time += dt;
		update_story_texts();
		update_decorations();
		update_controls_label();

		if (input.KeyJustPressed(Input::Key::Left) || input.KeyJustPressed('A'))
		{
			page_backward();
		}
		if (input.KeyJustPressed(Input::Key::Space) || input.KeyJustPressed(Input::Key::Right) || input.KeyJustPressed('D') || input.MouseButtonJustPressed(Input::MouseButton::Left))
		{
			const float reveal_end_time = get_current_page_reveal_end_time();
			if (m_page_time < reveal_end_time)
			{
				m_page_time = reveal_end_time;
				update_story_texts();
				update_decorations();
				update_controls_label();
			}
			else if (!page_forward())
			{
				return SceneTransition{ m_scene_data.next_scene_id, m_scene_id };
			}
		}

#ifdef _DEBUG
		if (input.KeyJustPressed('R'))
			reload_story();
#endif
	}

	m_fps_label.Update(dt);

	return std::nullopt;
}

void StoryScene::DestroyPendingAssets() const
{
	m_asset_manager.DestroyPendingAssets();
}

void StoryScene::SetTransitionOpacity(float opacity)
{
	m_scene_fade_overlay.SetOpacity(opacity);
}

void StoryScene::Render() const
{
	m_fps_label.RenderOffscreenTexture();
	m_controls_label.RenderOffscreenTexture();
	m_page_number_label.RenderOffscreenTexture();
	for (UIShadowedLabel const & story_label : m_story_labels)
		story_label.RenderOffscreenTexture();
	for (UIShadowedDecoration const & decoration : m_decorations)
		decoration.RenderOffscreenTexture();
	m_renderer.Render();
}

void StoryScene::ChangeSceneState(SceneState new_state)
{
	m_scene_state = new_state;
}

void StoryScene::pause()
{
	m_state_before_pause = m_scene_state;
	ChangeSceneState(SceneState::Paused);
	m_pause_overlay.SetVisible(true);
}

void StoryScene::resume()
{
	m_pause_overlay.SetVisible(false);
	ChangeSceneState(m_state_before_pause);
}

bool StoryScene::page_forward()
{
	if (m_page_index + 1 >= m_bg_tex_ids.size())
		return false;

	start_page_transition(static_cast<std::uint8_t>(m_page_index + 1));
	return true;
}

void StoryScene::page_backward()
{
	if (m_page_index == 0)
		return;

	start_page_transition(static_cast<std::uint8_t>(m_page_index - 1));
}

void StoryScene::start_page_transition(std::uint8_t page_index)
{
	m_pending_page_index = page_index;
	m_page_transition_state = PageTransitionState::FadingOut;
	m_page_transition_opacity = 0.0f;
	m_page_fade_overlay.SetOpacity(m_page_transition_opacity);
}

void StoryScene::update_page_transition(float dt)
{
	const float transition_step = std::max(dt, 0.0f) / PageTransitionDuration;
	if (m_page_transition_state == PageTransitionState::FadingOut)
	{
		m_page_transition_opacity = std::clamp(m_page_transition_opacity + transition_step, 0.0f, 1.0f);
		m_page_fade_overlay.SetOpacity(m_page_transition_opacity);
		if (m_page_transition_opacity >= 1.0f)
		{
			m_page_index = m_pending_page_index;
			apply_current_page();
			m_page_transition_state = PageTransitionState::FadingIn;
		}
		return;
	}

	m_page_transition_opacity = std::clamp(m_page_transition_opacity - transition_step, 0.0f, 1.0f);
	m_page_fade_overlay.SetOpacity(m_page_transition_opacity);
	if (m_page_transition_opacity <= 0.0f)
		m_page_transition_state = PageTransitionState::None;
}

std::filesystem::path StoryScene::get_story_filepath() const
{
	return m_asset_manager.GetResourcesPath() / "story" / (std::string{ ToString(m_scene_id) } + ".json");
}

void StoryScene::load_background_textures()
{
	std::vector<AssetId> old_bg_tex_ids = std::move(m_bg_tex_ids);
	m_bg_tex_ids.clear();
	std::ranges::transform(m_scene_data.pages, std::back_inserter(m_bg_tex_ids), [&](StoryPage const & page) {
		return m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "story_backgrounds" / page.bg_image_filename,
			dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	});

	for (AssetId tex_id : old_bg_tex_ids)
		m_asset_manager.RemoveTexture(tex_id);
}

void StoryScene::ensure_story_label_count(std::size_t count)
{
	const std::size_t old_size = m_story_labels.size();
	if (old_size >= count)
		return;

	m_story_labels.resize(count);
	for (std::size_t i = old_size; i < m_story_labels.size(); ++i)
	{
		UIShadowedLabel & story_label = m_story_labels[i];
		story_label.Init(
			m_asset_manager,
			m_renderer,
			m_camera2d,
			"story " + std::to_string(i + 1),
			"",
			m_font_atlas,
			TitleFontSize,
			glm::vec2{ 960, 250 } /*origin*/,
			UILabel::Align::Center,
			StoryTextColor);
		story_label.SetVisible(false);
	}
}

void StoryScene::ensure_decoration_count(std::size_t count)
{
	const std::size_t old_size = m_decorations.size();
	if (old_size >= count)
		return;

	m_decorations.resize(count);
	for (std::size_t i = old_size; i < m_decorations.size(); ++i)
	{
		UIShadowedDecoration & decoration = m_decorations[i];
		decoration.Init(
			m_asset_manager,
			m_renderer,
			m_camera2d,
			"story decoration " + std::to_string(i + 1),
			m_decoration_tex_id,
			Decorations::DecorationId::HorizontalDividerPawFlourish,
			glm::vec2{ 0.0f } /*center*/,
			1.0f /*scale*/,
			StoryTextColor);
		decoration.SetOpacity(0.0f);
		decoration.SetVisible(false);
	}
}

void StoryScene::ensure_story_ui_capacity()
{
	std::size_t max_story_texts = 0;
	for (StoryPage const & page : m_scene_data.pages)
		max_story_texts = std::max(max_story_texts, page.story_texts.size());

	std::size_t max_decorations = 0;
	for (StoryPage const & page : m_scene_data.pages)
		max_decorations = std::max(max_decorations, page.decorations.size());

	ensure_story_label_count(max_story_texts);
	ensure_decoration_count(max_decorations);
}

void StoryScene::reload_story()
{
	StorySceneData reloaded_scene_data = StoryLoader::LoadSceneData(get_story_filepath());
	if (reloaded_scene_data.pages.empty())
	{
		LOG(WARNING) << "StoryScene: Reload skipped because no pages loaded for story '" << ToString(m_scene_id) << "'.";
		return;
	}

	m_scene_data = std::move(reloaded_scene_data);
	load_background_textures();
	ensure_story_ui_capacity();

	const std::size_t cur_page_index = std::min<std::size_t>(m_page_index, m_scene_data.pages.size() - 1);
	m_page_index = static_cast<std::uint8_t>(cur_page_index);
	apply_current_page();
	LOG(INFO) << "StoryScene: Reloaded story '" << ToString(m_scene_id) << "'.";
}

void StoryScene::apply_current_page()
{
	m_page_time = 0.0f;
	m_background.SetTextureId(m_bg_tex_ids[m_page_index]);

	StoryPage const & page = m_scene_data.pages[m_page_index];
	for (std::size_t i = 0; i < m_story_labels.size(); ++i)
	{
		UIShadowedLabel & story_label = m_story_labels[i];
		if (i >= page.story_texts.size())
		{
			story_label.SetVisible(false);
			continue;
		}

		StoryText const & story_text = page.story_texts[i];
		story_label.SetText(story_text.text);
		story_label.SetFontSize(story_text.font_size);
		story_label.SetOrigin(story_text.pos);
		story_label.SetAlign(story_text.align);
		story_label.SetTextColor(story_text.color);
		story_label.SetOpacity(0.0f);
		story_label.SetVisible(true);
	}

	for (std::size_t i = 0; i < m_decorations.size(); ++i)
	{
		UIShadowedDecoration & decoration = m_decorations[i];
		if (i >= page.decorations.size())
		{
			decoration.SetVisible(false);
			continue;
		}

		StoryDecoration const & story_decoration = page.decorations[i];
		decoration.SetDecorationId(story_decoration.decoration_id);
		decoration.SetCenter(story_decoration.center);
		decoration.SetScale(story_decoration.scale);
		decoration.SetColor(story_decoration.color);
		decoration.SetOpacity(0.0f);
		decoration.SetVisible(true);
	}

	m_page_number_label.SetText(std::to_string(m_page_index + 1) + "/" + std::to_string(m_bg_tex_ids.size()));
	update_controls_label();
}

void StoryScene::update_story_texts()
{
	StoryPage const & page = m_scene_data.pages[m_page_index];
	for (std::size_t i = 0; i < page.story_texts.size(); ++i)
	{
		StoryText const & story_text = page.story_texts[i];
		const float fade_duration = std::max(story_text.fade_duration, 0.0f);
		const float opacity = fade_duration == 0.0f
			? (m_page_time >= story_text.show_time ? 1.0f : 0.0f)
			: std::clamp((m_page_time - story_text.show_time) / fade_duration, 0.0f, 1.0f);
		m_story_labels[i].SetOpacity(opacity);
	}
}

void StoryScene::update_decorations()
{
	StoryPage const & page = m_scene_data.pages[m_page_index];
	for (std::size_t i = 0; i < page.decorations.size(); ++i)
	{
		StoryDecoration const & decoration = page.decorations[i];
		const float fade_duration = std::max(decoration.fade_duration, 0.0f);
		const float opacity = fade_duration == 0.0f
			? (m_page_time >= decoration.show_time ? 1.0f : 0.0f)
			: std::clamp((m_page_time - decoration.show_time) / fade_duration, 0.0f, 1.0f);
		m_decorations[i].SetOpacity(opacity);
	}
}

void StoryScene::update_controls_label()
{
	constexpr float FadeDuration = 0.5f;

	const float reveal_start_time = get_current_page_reveal_end_time() + FadeDuration;
	const float opacity = std::clamp((m_page_time - reveal_start_time) / FadeDuration, 0.0f, 1.0f);
	m_controls_label.SetOpacity(opacity);
}

float StoryScene::get_current_page_reveal_end_time() const
{
	StoryPage const & page = m_scene_data.pages[m_page_index];
	float reveal_end_time = 0.0f;

	for (StoryText const & story_text : page.story_texts)
		reveal_end_time = std::max(reveal_end_time, story_text.show_time + std::max(story_text.fade_duration, 0.0f));

	for (StoryDecoration const & decoration : page.decorations)
		reveal_end_time = std::max(reveal_end_time, decoration.show_time + std::max(decoration.fade_duration, 0.0f));

	return reveal_end_time;
}

