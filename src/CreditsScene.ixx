// CreditsScene.ixx

module;

#include <optional>

#include <glm/vec2.hpp>

export module CreditsScene;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Background;
import Camera;
import ColorPipeline;
import FontAtlas;
import Input;
import IScene;
import SceneRenderer;
import SniffTheWayConstants;
import Texture2dPipeline;
import UIDarkBackdrop;
import UILabel;
import UIShadowedLabel;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class CreditsScene : public IScene
{
public:
	explicit CreditsScene(dh::RenderContext const & render_context);

	void OnWindowResized(int width, int height) override;
	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void DestroyPendingAssets() const override;
	void Render() const override;

private:
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera2d m_camera2d;
	Background m_background;
	UIDarkBackdrop m_dark_backdrop;
	FontAtlas m_font_atlas;
	UIShadowedLabel m_title_label;
	UIShadowedLabel m_creator_label;
	UIShadowedLabel m_thanks_label;
	UIShadowedLabel m_controls_label;
};

CreditsScene::CreditsScene(dh::RenderContext const & render_context)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
{
	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);

	const auto texture_pipeline_id = m_asset_manager.AddPipeline<Texture2dPipeline>(m_camera2d, m_asset_manager);
	const auto color_pipeline_id = m_asset_manager.AddPipeline<ColorPipeline>(m_camera2d);

	const AssetId background_texture_id = m_asset_manager.AddTexture(
		m_asset_manager.GetTexturesPath() / "menus" / "title_forest_path.png",
		dh::PixelFormat::RGBA_SRGB,
		false /*flip_vertically*/,
		false /*use_mip_map*/);
	m_background.Init(m_asset_manager, background_texture_id);
	m_renderer.CreateRenderObject(
		"credits background",
		RenderLayer::Background,
		m_background.GetMeshId(),
		texture_pipeline_id,
		m_background.GetPipelineData());

	m_dark_backdrop.Init(m_asset_manager, 0.0f, UIWidth, 0.0f, UIHeight, 0.4f, 0.7f);
	m_dark_backdrop.SetROId(m_renderer.CreateRenderObject(
		"credits dark backdrop",
		RenderLayer::UIShadow,
		m_dark_backdrop.GetMeshId(),
		color_pipeline_id));

	const AssetId font_texture_id = m_asset_manager.AddTexture(
		m_asset_manager.GetFontsPath() / "Alice.png",
		dh::PixelFormat::RGB_UNORM,
		true /*flip_vertically*/,
		false /*use_mip_map*/);
	m_font_atlas.Init(font_texture_id, m_asset_manager.GetFontsPath() / "Alice.json");

	m_title_label.Init(
		m_asset_manager,
		m_renderer,
		m_camera2d,
		"credits title",
		"CREDITS",
		m_font_atlas,
		StoryLargeFontSize,
		glm::vec2{ UIWidth * 0.5f, 250.0f },
		UILabel::Align::Center,
		StoryTextColor);

	m_creator_label.Init(
		m_asset_manager,
		m_renderer,
		m_camera2d,
		"credits creator",
		"Created by\nJonathan Kraber",
		m_font_atlas,
		StoryMediumFontSize,
		glm::vec2{ UIWidth * 0.5f, 440.0f },
		UILabel::Align::Center,
		StoryTextColor);

	m_thanks_label.Init(
		m_asset_manager,
		m_renderer,
		m_camera2d,
		"credits thanks",
		"Special thanks to my wife Rayne",
		m_font_atlas,
		StoryMediumFontSize,
		glm::vec2{ UIWidth * 0.5f, 700.0f },
		UILabel::Align::Center,
		StoryTextColor);

	m_controls_label.Init(
		m_asset_manager,
		m_renderer,
		m_camera2d,
		"credits controls",
		"(Press [Esc] to return)",
		m_font_atlas,
		LabelFontSize,
		glm::vec2{ UIWidth * 0.5f, 1026.0f },
		UILabel::Align::Center,
		StoryTextColor);
}

void CreditsScene::OnWindowResized(int width, int height)
{
	const int game_width = static_cast<int>(height * 16.0f / 9.0f);
	const int x_offset = (width - game_width) / 2;
	m_renderer.SetViewport(x_offset, 0, game_width, height);
	m_camera2d.SetViewportSize(game_width, height);
}

std::optional<SceneTransition> CreditsScene::Update(float /*dt*/, Input const & input)
{
	if (input.KeyJustPressed(Input::Key::Esc)
		|| input.KeyJustPressed(Input::Key::Space)
		|| (input.KeyJustPressed(Input::Key::Enter) && !input.AltIsDown()))
	{
		return SceneTransition{ SceneId::Title, SceneId::Credits };
	}

	return std::nullopt;
}

void CreditsScene::DestroyPendingAssets() const
{
	m_asset_manager.DestroyPendingAssets();
}

void CreditsScene::Render() const
{
	m_title_label.RenderOffscreenTexture();
	m_creator_label.RenderOffscreenTexture();
	m_thanks_label.RenderOffscreenTexture();
	m_controls_label.RenderOffscreenTexture();
	m_renderer.Render();
}
