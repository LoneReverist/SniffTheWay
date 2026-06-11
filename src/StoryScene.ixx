// StoryScene.ixx

module;

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

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
import Input;
import IScene;
import SceneRenderer;
import SniffTheWayConstants;
import UILabel;
import UIShadowedDecoration;
import UIShadowedLabel;
import Vertex;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export struct StoryText
{
	std::string_view text;
	float font_size = LabelFontSize;
	glm::vec2 pos{ 960.0f, 250.0f };
	UILabel::Align align = UILabel::Align::Center;
	float show_time = 0.0f;
	float fade_duration = 0.5f;
	glm::vec4 color = StoryTextColor;
};

export struct StoryDecoration
{
	Decorations::DecorationId decoration_id;
	glm::vec2 center;
	float scale = 1.5f;
	float show_time = 0.0f;
	float fade_duration = 0.5f;
	glm::vec4 color = StoryTextColor;
};

export struct StoryPage
{
	std::string_view bg_image_filename;
	std::span<StoryText const> story_texts;
	std::span<StoryDecoration const> decorations;
};

export class StoryScene : public IScene
{
public:
	explicit StoryScene(
		dh::RenderContext const & render_context,
		std::span<StoryPage const> story_pages,
		SceneId next_scene_id);

	void OnWindowResized(int width, int height) override;

	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void DestroyPendingAssets() const override;
	void Render() const override;

	void ChangeSceneState(SceneState new_state);

private:
	bool page_forward();
	void page_backward();
	void apply_current_page();
	void update_story_texts();
	void update_decorations();

private:
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera2d m_camera2d;
	SceneState m_scene_state = SceneState::Paused;
	SceneId m_next_scene_id = SceneId::Exit;
	std::span<StoryPage const> m_story_pages;
	float m_page_time = 0.0f;

	Background m_background;
	std::vector<AssetId> m_bg_tex_ids;
	std::uint8_t m_cur_bg_index = 0;

	std::unique_ptr<FontAtlas> m_font_atlas;
	FPSLabel m_fps_label;
	UIShadowedLabel m_controls_label;
	UIShadowedLabel m_page_number_label;
	std::vector<UIShadowedLabel> m_story_labels;
	std::vector<UIShadowedDecoration> m_decorations;
};

StoryScene::StoryScene(
	dh::RenderContext const & render_context,
	std::span<StoryPage const> story_pages,
	SceneId next_scene_id)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
	, m_story_pages{ story_pages }
	, m_next_scene_id{ next_scene_id }
{
	const auto bg_pipeline_id = m_asset_manager.AddPipeline<Texture2dPipeline>(m_camera2d, m_asset_manager);

	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);

	std::ranges::transform(m_story_pages, std::back_inserter(m_bg_tex_ids), [&](StoryPage const & page) {
		return m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "story_backgrounds" / page.bg_image_filename,
			dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	});
	m_cur_bg_index = 0;
	m_background.Init(m_asset_manager, m_bg_tex_ids[m_cur_bg_index]);
	m_renderer.CreateRenderObject("background", RenderLayer::Background, m_background.GetMeshId(), bg_pipeline_id, m_background.GetPipelineData());

	AssetId decoration_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / Decorations::TextureFileName,
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	AssetId font_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetFontsPath() / "Alice.png",
		dh::PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_font_atlas = std::make_unique<FontAtlas>(font_tex_id, m_asset_manager.GetFontsPath() / "Alice.json");

	m_fps_label.Init(m_asset_manager, m_renderer, m_camera2d, *m_font_atlas);

	m_controls_label.Init(
		m_asset_manager,
		m_renderer,
		m_camera2d,
		"controls",
		"(Press [Space] to continue)",
		*m_font_atlas,
		LabelFontSize,
		glm::vec2{ 960, 1026 } /*origin*/,
		UILabel::Align::Center,
		StoryTextColor);

	std::string page_number_text = std::to_string(m_cur_bg_index + 1) + "/" + std::to_string(m_bg_tex_ids.size());
	m_page_number_label.Init(
		m_asset_manager,
		m_renderer,
		m_camera2d,
		"page number",
		page_number_text,
		*m_font_atlas,
		LabelFontSize,
		glm::vec2{ 1824, 1026 } /*origin*/,
		UILabel::Align::Right,
		StoryTextColor);

	std::size_t max_story_texts = 0;
	for (StoryPage const & page : m_story_pages)
		max_story_texts = std::max(max_story_texts, page.story_texts.size());

	std::size_t max_decorations = 0;
	for (StoryPage const & page : m_story_pages)
		max_decorations = std::max(max_decorations, page.decorations.size());

	m_story_labels.resize(max_story_texts);
	for (std::size_t i = 0; i < m_story_labels.size(); ++i)
	{
		UIShadowedLabel & story_label = m_story_labels[i];
		story_label.Init(
			m_asset_manager,
			m_renderer,
			m_camera2d,
			"story " + std::to_string(i + 1),
			"",
			*m_font_atlas,
			TitleFontSize,
			glm::vec2{ 960, 250 } /*origin*/,
			UILabel::Align::Center,
			StoryTextColor);
		story_label.SetVisible(false);
	}

	m_decorations.resize(max_decorations);
	for (std::size_t i = 0; i < m_decorations.size(); ++i)
	{
		UIShadowedDecoration & decoration = m_decorations[i];
		decoration.Init(
			m_asset_manager,
			m_renderer,
			m_camera2d,
			"story decoration " + std::to_string(i + 1),
			decoration_tex_id,
			Decorations::DecorationId::HorizontalDividerPawFlourish,
			glm::vec2{ 0.0f } /*center*/,
			1.0f /*scale*/,
			StoryTextColor);
		decoration.SetOpacity(0.0f);
		decoration.SetVisible(false);
	}

	apply_current_page();

	ChangeSceneState(SceneState::Story);
}

void StoryScene::OnWindowResized(int width, int height)
{
	int game_width = static_cast<int>(height * 16.0f / 9.0f);
	int x_offset = (width - game_width) / 2;
	m_renderer.SetViewport(x_offset, 0, game_width, height);

	m_camera2d.SetViewportSize(game_width, height);
}

std::optional<SceneTransition> StoryScene::Update(float dt, Input const & input)
{
	if (input.KeyJustPressed(Input::Key::Esc))
		return SceneTransition{ SceneId::Exit };

	if (m_scene_state == SceneState::Story)
	{
		m_page_time += dt;
		update_story_texts();
		update_decorations();

		if (input.KeyJustPressed(Input::Key::Left) || input.KeyJustPressed('A'))
		{
			page_backward();
		}
		if (input.KeyJustPressed(Input::Key::Space) || input.KeyJustPressed(Input::Key::Right) || input.KeyJustPressed('D'))
		{
			if (!page_forward())
				return SceneTransition{ m_next_scene_id };
		}
	}

	m_fps_label.Update(dt);

	return std::nullopt;
}

void StoryScene::DestroyPendingAssets() const
{
	m_asset_manager.DestroyPendingAssets();
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

bool StoryScene::page_forward()
{
	if (m_cur_bg_index + 1 >= m_bg_tex_ids.size())
		return false;

	m_cur_bg_index++;
	apply_current_page();
	return true;
}

void StoryScene::page_backward()
{
	if (m_cur_bg_index > 0)
		m_cur_bg_index--;

	apply_current_page();
}

void StoryScene::apply_current_page()
{
	m_page_time = 0.0f;
	m_background.SetTextureId(m_bg_tex_ids[m_cur_bg_index]);

	StoryPage const & page = m_story_pages[m_cur_bg_index];
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

	m_page_number_label.SetText(std::to_string(m_cur_bg_index + 1) + "/" + std::to_string(m_bg_tex_ids.size()));
}

void StoryScene::update_story_texts()
{
	StoryPage const & page = m_story_pages[m_cur_bg_index];
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
	StoryPage const & page = m_story_pages[m_cur_bg_index];
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

