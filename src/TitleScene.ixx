// TitleScene.ixx

module;

#include <optional>

#include <glm/vec2.hpp>

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
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera2d m_camera2d;
	Background m_background;
	UIImage m_title_image;
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
}

void TitleScene::OnWindowResized(int width, int height)
{
	const int game_width = static_cast<int>(height * 16.0f / 9.0f);
	const int x_offset = (width - game_width) / 2;
	m_renderer.SetViewport(x_offset, 0, game_width, height);
	m_camera2d.SetViewportSize(game_width, height);
}

std::optional<SceneTransition> TitleScene::Update(float /*dt*/, Input const & input)
{
	if (input.KeyJustPressed(Input::Key::Esc))
		return SceneTransition{ SceneId::Exit };

	if (input.KeyJustPressed(Input::Key::Enter) && !input.AltIsDown())
		return SceneTransition{ SceneId::Picnic, SceneId::Title };

	return std::nullopt;
}

void TitleScene::DestroyPendingAssets() const
{
	m_asset_manager.DestroyPendingAssets();
}

void TitleScene::Render() const
{
	m_renderer.Render();
}
