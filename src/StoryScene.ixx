// StoryScene.ixx

module;

#include <filesystem>
#include <memory>
#include <optional>
#include <span>

#include <glm/glm.hpp>

export module StoryScene;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import AssetPool;
import Background;
import BackgroundTexPipeline;
import Camera;
import FontAtlas;
import FPSLabel;
import Input;
import IScene;
import SceneRenderer;
import SniffTheWayConstants;
import UILabel;
import UIShadowedText;
import Vertex;

using namespace SniffTheWay;

export class StoryScene : public IScene
{
public:
	explicit StoryScene(
		dh::RenderContext const & render_context,
		std::span<std::string_view const> bg_image_filenames,
		std::span<std::string_view const> story_texts);

	void OnWindowResized(int width, int height) override;

	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void Render() const override;

	void ChangeSceneState(SceneState new_state);

protected:
	bool page_forward();
	void page_backward();

protected:
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera2d m_camera2d;
	SceneState m_scene_state = SceneState::Paused;
	SceneId m_next_scene_id = SceneId::Exit;
	std::span<std::string_view const> m_story_texts;

	Background m_background;
	std::vector<AssetId> m_bg_tex_ids;
	std::uint8_t m_cur_bg_index = 0;

	std::unique_ptr<FontAtlas> m_arial_font;
	FPSLabel m_fps_label;
	UIShadowedText m_controls_label;
	UIShadowedText m_page_number_label;
	UIShadowedText m_story_text;
};

StoryScene::StoryScene(
	dh::RenderContext const & render_context,
	std::span<std::string_view const> bg_image_filenames,
	std::span<std::string_view const> story_texts)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
	, m_story_texts{ story_texts }
{
	const auto bg_pipeline_id = m_asset_manager.AddPipeline<BackgroundTexPipeline>(m_camera2d, m_asset_manager);

	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);

	// background
	std::ranges::transform(bg_image_filenames, std::back_inserter(m_bg_tex_ids), [&](std::string_view filename) {
		return m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "story_backgrounds" / filename,
			dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	});
	m_cur_bg_index = 0;
	m_background.Init(m_asset_manager, m_bg_tex_ids[m_cur_bg_index]);
	m_renderer.CreateRenderObject("background", m_background.GetMeshId(), bg_pipeline_id, m_background.GetPipelineData());

	// ui
	AssetId arial_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetFontsPath() / "ArialAtlas.png",
		dh::PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_arial_font = std::make_unique<FontAtlas>(arial_tex_id, m_asset_manager.GetFontsPath() / "ArialAtlas.json");

	m_fps_label.Init(m_asset_manager, m_renderer, m_camera2d, *m_arial_font);

	m_controls_label.Init(
		m_asset_manager,
		m_renderer,
		m_camera2d,
		"controls",
		"(Press [Space] to continue)",
		*m_arial_font,
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
		*m_arial_font,
		LabelFontSize,
		glm::vec2{ 1824, 1026 } /*origin*/,
		UILabel::Align::Right,
		StoryTextColor);

	m_story_text.Init(
		m_asset_manager,
		m_renderer,
		m_camera2d,
		"story",
		m_story_texts[m_cur_bg_index],
		*m_arial_font,
		TitleFontSize,
		glm::vec2{ 960, 250 } /*origin*/,
		UILabel::Align::Center,
		StoryTextColor);

	ChangeSceneState(SceneState::Story);
}

// override
void StoryScene::OnWindowResized(int width, int height)
{
	int game_width = static_cast<int>(height * 16.0f / 9.0f);
	int x_offset = (width - game_width) / 2;
	m_renderer.SetViewport(x_offset, 0, game_width, height);

	m_camera2d.SetViewportSize(game_width, height);
}

// override
std::optional<SceneTransition> StoryScene::Update(float dt, Input const & input)
{
	if (input.KeyJustPressed(Input::Key::Esc))
		return SceneTransition{ SceneId::Exit };

	if (m_scene_state == SceneState::Story)
	{
		if (input.KeyJustPressed(Input::Key::Left) || input.KeyJustPressed('A'))
		{
			page_backward();
		}
		if (input.KeyJustPressed(Input::Key::Space) || input.KeyJustPressed(Input::Key::Right) || input.KeyJustPressed('D'))
		{
			if (!page_forward())
				return SceneTransition{m_next_scene_id};
		}
	}

	m_fps_label.Update(dt);

	return std::nullopt;
}

// override
void StoryScene::Render() const
{
	m_fps_label.RenderOffscreenTexture();
	m_controls_label.RenderOffscreenTexture();
	m_page_number_label.RenderOffscreenTexture();
	m_story_text.RenderOffscreenTexture();
	m_renderer.Render();
}

void StoryScene::ChangeSceneState(SceneState new_state)
{
	m_scene_state = new_state;
}

bool StoryScene::page_forward()
{
	m_cur_bg_index++;
	if (m_cur_bg_index >= m_bg_tex_ids.size())
		return false;

	m_background.SetTextureId(m_bg_tex_ids[m_cur_bg_index]);
	m_story_text.SetText(m_story_texts[m_cur_bg_index]);
	m_page_number_label.SetText(std::to_string(m_cur_bg_index + 1) + "/" + std::to_string(m_bg_tex_ids.size()));
	return true;
}

void StoryScene::page_backward()
{
	m_cur_bg_index = std::max(0, m_cur_bg_index - 1);
	m_background.SetTextureId(m_bg_tex_ids[m_cur_bg_index]);
	m_story_text.SetText(m_story_texts[m_cur_bg_index]);
	m_page_number_label.SetText(std::to_string(m_cur_bg_index + 1) + "/" + std::to_string(m_bg_tex_ids.size()));
}
