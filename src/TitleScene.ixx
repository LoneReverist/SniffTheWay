// TitleScene.ixx

module;

#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

export module TitleScene;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Background;
import Camera;
import IScene;
import Input;
import SceneRenderer;
import SniffTheWayConstants;
import Texture2dPipeline;
import UIButton;
import UIImage;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class TitleScene : public IScene
{
public:
	explicit TitleScene(dh::RenderContext const & render_context);

	void OnWindowResized(int width, int height) override;
	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void DestroyPendingAssets() const override;
	void Render() const override;

private:
	std::optional<glm::vec2> screen_to_ui(glm::vec2 screen_position) const;

private:
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera2d m_camera2d;
	glm::ivec4 m_viewport{ 0 };
	Background m_background;
	UIImage m_title_image;
	UIButton m_start_button;
};

TitleScene::TitleScene(dh::RenderContext const & render_context)
	: m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
{
	m_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);

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
}

void TitleScene::OnWindowResized(int width, int height)
{
	const int game_width = static_cast<int>(height * 16.0f / 9.0f);
	const int x_offset = (width - game_width) / 2;
	m_viewport = glm::ivec4{ x_offset, 0, game_width, height };
	m_renderer.SetViewport(x_offset, 0, game_width, height);
	m_camera2d.SetViewportSize(game_width, height);
}

std::optional<SceneTransition> TitleScene::Update(float /*dt*/, Input const & input)
{
	if (input.KeyJustPressed(Input::Key::Esc))
		return SceneTransition{ SceneId::Exit };

	m_start_button.Update(input, screen_to_ui(input.GetMousePos()));
	if ((input.KeyJustPressed(Input::Key::Enter) && !input.AltIsDown()) || m_start_button.WasActivated())
		return SceneTransition{ SceneId::Picnic, SceneId::Title };

	return std::nullopt;
}

std::optional<glm::vec2> TitleScene::screen_to_ui(glm::vec2 screen_position) const
{
	if (m_viewport.z <= 0 || m_viewport.w <= 0
		|| screen_position.x < static_cast<float>(m_viewport.x)
		|| screen_position.x > static_cast<float>(m_viewport.x + m_viewport.z)
		|| screen_position.y < static_cast<float>(m_viewport.y)
		|| screen_position.y > static_cast<float>(m_viewport.y + m_viewport.w))
	{
		return std::nullopt;
	}

	return glm::vec2{
		(screen_position.x - static_cast<float>(m_viewport.x)) * UIWidth / static_cast<float>(m_viewport.z),
		(screen_position.y - static_cast<float>(m_viewport.y)) * UIHeight / static_cast<float>(m_viewport.w),
	};
}

void TitleScene::DestroyPendingAssets() const
{
	m_asset_manager.DestroyPendingAssets();
}

void TitleScene::Render() const
{
	m_renderer.Render();
}
