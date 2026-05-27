// ScenePicnic.ixx

module;

#include <filesystem>
#include <memory>
#include <optional>

#include <glm/glm.hpp>

export module ScenePicnic;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import AssetPool;
import Baby;
import Background;
import BackgroundTexPipeline;
import Camera;
import ColorPipeline;
import Dog;
import EditorGrid;
import FontAtlas;
import FPSLabel;
import Input;
import IScene;
import LinePipeline;
import Polygon2d;
import SceneRenderer;
import SniffTheWayConstants;
import SpritePipeline;
import TextPipeline;
import UILabel;
import UIShadow;
import Vertex;

using namespace SniffTheWay;

export class ScenePicnic : public IScene
{
public:
	explicit ScenePicnic(dh::RenderContext const & render_context);

	void OnViewportResized(int width, int height) override;
	void OnDPIScaleFactorChanged(float dpi_scale_factor) override;

	std::optional<SceneTransition> Update(float dt, Input const & input) override;
	void Render() const override;

	void ChangeSceneState(SceneState new_state);

private:
	dh::RenderContext const & m_render_context;
	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera2d m_camera2d;
	SceneState m_scene_state = SceneState::Paused;

	Background m_background;
	std::vector<AssetId> m_bg_tex_ids;
	std::uint8_t m_cur_bg_index = 0;

	std::unique_ptr<FontAtlas> m_arial_font;
	FPSLabel m_fps_label;
	std::unique_ptr<UILabel> m_story_label;
	UIShadow m_story_shadow;
};

ScenePicnic::ScenePicnic(dh::RenderContext const & render_context)
	: m_render_context{ render_context }
	, m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
{
	const auto bg_pipeline_id = m_asset_manager.AddPipeline<BackgroundTexPipeline>(m_camera2d, m_asset_manager);
	const auto color_pipeline_id = m_asset_manager.AddPipeline<ColorPipeline>(m_camera2d);
	const auto text_pipeline_id = m_asset_manager.AddPipeline<TextPipeline>(m_camera2d, m_asset_manager);

	// background
	AssetId bg_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "picnic.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	AssetId bg_tex_id2 = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "gust_of_wind.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	AssetId bg_tex_id3 = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "following_butterflies.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	AssetId bg_tex_id4 = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "lost.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	m_bg_tex_ids = { bg_tex_id, bg_tex_id2, bg_tex_id3, bg_tex_id4 };
	m_cur_bg_index = 0;
	m_background.Init(m_asset_manager, m_bg_tex_ids[m_cur_bg_index]);
	m_renderer.CreateRenderObject("background", m_background.GetMeshId(), bg_pipeline_id, m_background.GetPipelineData());

	// ui
	AssetId arial_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetFontsPath() / "ArialAtlas.png",
		dh::PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_arial_font = std::make_unique<FontAtlas>(arial_tex_id, m_asset_manager.GetFontsPath() / "ArialAtlas.json");

	m_story_shadow.Init(m_asset_manager, -1.0 /*left*/, 1.0 /*right*/, -0.6 /*top*/, -1.0 /*bottom*/);
	m_story_shadow.SetROId(m_renderer.CreateRenderObject("story shadow", m_story_shadow.GetMeshId(), color_pipeline_id));

	m_fps_label.Init(m_asset_manager, *m_arial_font);
	m_renderer.CreateRenderObject("fps label", m_fps_label.GetUILabel()->GetMeshId(), text_pipeline_id, m_fps_label.GetUILabel()->GetPipelineData());

	m_story_label = std::make_unique<UILabel>(m_asset_manager, "(Press [Space] to continue)", *m_arial_font,
		LabelFontSize, glm::vec2{ 0.0, -0.8 } /*origin*/, UILabel::Align::Center, StoryTextColor);
	m_story_label->SetROId(m_renderer.CreateRenderObject("story label", m_story_label->GetMeshId(), text_pipeline_id, m_story_label->GetPipelineData()));

	ChangeSceneState(SceneState::Story);
}

// override
void ScenePicnic::OnViewportResized(int width, int height)
{
	m_camera2d.OnViewportResized(width, height);

	// keep UI elements proportional to the height of the view
	m_background.OnViewportResized(width, height, m_asset_manager);
	m_fps_label.OnViewportResized(width, height);
	if (m_story_label)
		m_story_label->OnViewportResized(width, height);
}

// override
void ScenePicnic::OnDPIScaleFactorChanged(float dpi_scale_factor)
{
	m_fps_label.OnDPIScaleFactorChanged(dpi_scale_factor);
	if (m_story_label)
		m_story_label->SetFontSize(LabelFontSize * dpi_scale_factor);
}

// override
std::optional<SceneTransition> ScenePicnic::Update(float dt, Input const & input)
{
	if (input.KeyJustPressed(Input::Key::Esc))
		return SceneTransition{ SceneId::Exit };

	if (m_scene_state == SceneState::Story && input.KeyJustPressed(Input::Key::Space))
	{
		m_cur_bg_index++;
		if (m_cur_bg_index >= m_bg_tex_ids.size())
			return SceneTransition{ SceneId::ForestPath };

		m_background.SetTextureId(m_bg_tex_ids[m_cur_bg_index]);
	}

	m_fps_label.Update(dt);

	return std::nullopt;
}

// override
void ScenePicnic::Render() const
{
	m_renderer.Render();
}

void ScenePicnic::ChangeSceneState(SceneState new_state)
{
	m_scene_state = new_state;
}
