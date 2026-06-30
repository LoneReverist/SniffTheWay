// TitleScene.ixx

module;

#include <optional>

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

	if (input.KeyJustPressed(Input::Key::Enter))
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
