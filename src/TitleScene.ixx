// TitleScene.ixx

module;

#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

export module TitleScene;

import Dreamhearth;

import AssetManager;
import AssetPool;
import AudioSystem;
import Background;
import Camera;
import GameViewport;
import FontAtlas;
import IScene;
import Input;
import SceneRenderer;
import SceneFadeOverlay;
import SettingsOverlay;
import SniffTheWayConstants;
import Texture2dPipeline;
import UIButton;
import UIImage;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class TitleScene : public IScene
{
public:
	explicit TitleScene(dh::RenderContext const & render_context, AudioSystem & audio_system);

	void OnViewportChanged(GameViewport const & viewport) override;
	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void DestroyPendingAssets() const override;
	void SetTransitionOpacity(float opacity) override;
	void Render() const override;

private:
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera2d m_camera2d;
	GameViewport m_viewport;
	AudioSystem * m_audio_system = nullptr;
	FontAtlas m_font_atlas;
	Background m_background;
	UIImage m_dog_image;
	UIImage m_baby_image;
	UIImage m_title_image;
	UIButton m_start_button;
	UIButton m_settings_button;
	UIButton m_credits_button;
	SettingsOverlay m_settings_overlay;
	SceneFadeOverlay m_scene_fade_overlay;
};

TitleScene::TitleScene(dh::RenderContext const & render_context, AudioSystem & audio_system)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
{
	m_audio_system = &audio_system;
	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);
	const AssetId font_texture_id = m_asset_manager.AddTexture(
		m_asset_manager.GetFontsPath() / "Alice.png",
		dh::PixelFormat::RGB_UNORM,
		true /*flip_vertically*/,
		false /*use_mip_map*/);
	m_font_atlas.Init(font_texture_id, m_asset_manager.GetFontsPath() / "Alice.json");

	const auto texture_pipeline_id = m_asset_manager.AddPipeline<Texture2dPipeline>(m_camera2d, m_asset_manager);
	const AssetId background_texture_id = m_asset_manager.AddTexture(
		m_asset_manager.GetTexturesPath() / "menus" / "title_forest_path.png",
		dh::PixelFormat::RGBA_SRGB,
		false /*flip_vertically*/,
		false /*use_mip_map*/);

	m_background.Init(m_asset_manager, background_texture_id);
	m_renderer.CreateRenderObject(
		"title background",
		RenderLayer::Background,
		m_background.GetMeshId(),
		texture_pipeline_id,
		m_background.GetPipelineData());

	const AssetId baby_texture_id = m_asset_manager.AddTexture(
		m_asset_manager.GetTexturesPath() / "baby_crawl.png",
		dh::PixelFormat::RGBA_SRGB,
		false /*flip_vertically*/,
		false /*use_mip_map*/);
	m_baby_image.Init(
		m_asset_manager,
		baby_texture_id,
		glm::vec2{ 1150.0f, 580.0f } /*center*/,
		glm::vec2{ 270.0f, 270.0f } /*size*/,
		UIImage::UVBounds{ 1.0f / 3.0f, 2.0f / 3.0f, 2.0f / 3.0f, 1.0f } /*frame 7*/);
	m_renderer.CreateRenderObject(
		"title baby",
		RenderLayer::Scene3d,
		m_baby_image.GetMeshId(),
		texture_pipeline_id,
		m_baby_image.GetPipelineData());

	const AssetId dog_texture_id = m_asset_manager.AddTexture(
		m_asset_manager.GetTexturesPath() / "dog_walk.png",
		dh::PixelFormat::RGBA_SRGB,
		false /*flip_vertically*/,
		false /*use_mip_map*/);
	m_dog_image.Init(
		m_asset_manager,
		dog_texture_id,
		glm::vec2{ 900.0f, 655.0f } /*center*/,
		glm::vec2{ 390.0f, 455.0f } /*size*/,
		UIImage::UVBounds{ 0.5f, 0.75f, 0.0f, 0.5f } /*frame 2*/);
	m_renderer.CreateRenderObject(
		"title dog",
		RenderLayer::Scene3d,
		m_dog_image.GetMeshId(),
		texture_pipeline_id,
		m_dog_image.GetPipelineData());

	const AssetId title_texture_id = m_asset_manager.AddTexture(
		m_asset_manager.GetTexturesPath() / "menus" / "title.png",
		dh::PixelFormat::RGBA_SRGB,
		false /*flip_vertically*/,
		false /*use_mip_map*/);

	constexpr float title_scale = 0.75f;
	m_title_image.Init(
		m_asset_manager,
		title_texture_id,
		glm::vec2{ UIWidth * 0.5f, 230.0f } /*center*/,
		glm::vec2{ 1642.0f, 540.0f } * title_scale /*size*/);
	m_renderer.CreateRenderObject(
		"title",
		RenderLayer::UIForeground,
		m_title_image.GetMeshId(),
		texture_pipeline_id,
		m_title_image.GetPipelineData());

	const AssetId start_button_texture_id = m_asset_manager.AddTexture(
		m_asset_manager.GetTexturesPath() / "menus" / "start_adventure.png",
		dh::PixelFormat::RGBA_SRGB,
		false /*flip_vertically*/,
		false /*use_mip_map*/);

	m_start_button.Init(
		m_asset_manager,
		start_button_texture_id,
		glm::vec2{ UIWidth * 0.5f, 950.0f } /*center*/,
		glm::vec2{ 694.0f, 164.0f } /*size*/,
		UIButton::Frames{
			.normal = UIImage::UVBounds{ 0.0f, 1.0f / 3.0f, 0.0f, 1.0f },
			.hovered = UIImage::UVBounds{ 1.0f / 3.0f, 2.0f / 3.0f, 0.0f, 1.0f },
			.pressed = UIImage::UVBounds{ 2.0f / 3.0f, 1.0f, 0.0f, 1.0f },
		});
	m_renderer.CreateRenderObject(
		"start adventure",
		RenderLayer::UIForeground,
		m_start_button.GetMeshId(),
		texture_pipeline_id,
		m_start_button.GetPipelineData());

	const AssetId side_buttons_texture_id = m_asset_manager.AddTexture(
		m_asset_manager.GetTexturesPath() / "menus" / "buttons.png",
		dh::PixelFormat::RGBA_SRGB,
		false /*flip_vertically*/,
		false /*use_mip_map*/);

	m_settings_button.Init(
		m_asset_manager,
		side_buttons_texture_id,
		glm::vec2{ 140.0f, 960.0f } /*center*/,
		glm::vec2{ 160.0f, 180.0f } /*size*/,
		UIButton::Frames{
			.normal = UIImage::UVBounds{ 0.0f, 1.0f / 3.0f, 0.0f, 0.5f },
			.hovered = UIImage::UVBounds{ 1.0f / 3.0f, 2.0f / 3.0f, 0.0f, 0.5f },
			.pressed = UIImage::UVBounds{ 2.0f / 3.0f, 1.0f, 0.0f, 0.5f },
		});
	m_renderer.CreateRenderObject(
		"settings",
		RenderLayer::UIForeground,
		m_settings_button.GetMeshId(),
		texture_pipeline_id,
		m_settings_button.GetPipelineData());

	m_credits_button.Init(
		m_asset_manager,
		side_buttons_texture_id,
		glm::vec2{ UIWidth - 140.0f, 960.0f } /*center*/,
		glm::vec2{ 160.0f, 180.0f } /*size*/,
		UIButton::Frames{
			.normal = UIImage::UVBounds{ 0.0f, 1.0f / 3.0f, 0.5f, 1.0f },
			.hovered = UIImage::UVBounds{ 1.0f / 3.0f, 2.0f / 3.0f, 0.5f, 1.0f },
			.pressed = UIImage::UVBounds{ 2.0f / 3.0f, 1.0f, 0.5f, 1.0f },
		});
	m_renderer.CreateRenderObject(
		"credits",
		RenderLayer::UIForeground,
		m_credits_button.GetMeshId(),
		texture_pipeline_id,
		m_credits_button.GetPipelineData());

	m_settings_overlay.Init(m_asset_manager, m_renderer, m_camera2d, m_font_atlas, audio_system);
	m_scene_fade_overlay.Init(m_asset_manager, m_renderer, m_camera2d);
}

void TitleScene::OnViewportChanged(GameViewport const & viewport)
{
	m_viewport = viewport;
	const auto & pixels = viewport.pixels;
	m_renderer.SetViewport(pixels.x, pixels.y, pixels.z, pixels.w);
	m_camera2d.SetViewportSize(pixels.z, pixels.w);
}

std::optional<SceneTransition> TitleScene::Update(float /*dt*/, Input const & input)
{
	if (m_settings_overlay.IsVisible())
	{
		if (m_settings_overlay.Update(input, m_viewport))
			m_settings_overlay.SetVisible(false);
		return std::nullopt;
	}

	if (input.KeyJustPressed(Input::Key::Esc))
		return SceneTransition{ SceneId::Exit };

	const std::optional<glm::vec2> pointer_position = m_viewport.FramebufferToUI(input.GetMousePos());
	m_start_button.Update(input, pointer_position);
	m_settings_button.Update(input, pointer_position);
	m_credits_button.Update(input, pointer_position);
	if ((input.KeyJustPressed(Input::Key::Enter) && !input.AltIsDown()) || m_start_button.WasActivated())
	{
		m_audio_system->PlaySound(SoundCue::ShortChime);
		return SceneTransition{ SceneId::Picnic, SceneId::Title, PlaythroughAction::StartNew };
	}
	if (m_credits_button.WasActivated())
	{
		m_audio_system->PlaySound(SoundCue::ShortChime);
		return SceneTransition{ SceneId::Credits, SceneId::Title };
	}
	if (m_settings_button.WasActivated())
	{
		m_audio_system->PlaySound(SoundCue::ShortChime);
		m_settings_overlay.SetVisible(true);
	}

	return std::nullopt;
}

void TitleScene::DestroyPendingAssets() const
{
	m_asset_manager.DestroyPendingAssets();
}

void TitleScene::SetTransitionOpacity(float opacity)
{
	m_scene_fade_overlay.SetOpacity(opacity);
}

void TitleScene::Render() const
{
	m_renderer.Render();
}
