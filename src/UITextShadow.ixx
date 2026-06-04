// UITextShadow.ixx

module;

#include <expected>
#include <iostream>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

export module UITextShadow;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import BlurPipeline;
import Camera;
import FontAtlas;
import RenderObject;
import SceneRenderer;
import ShadowCompositePipeline;
import SniffTheWayConstants;
import TextMaskPipeline;
import TextPipeline;
import UILabel;
import Vertex;

using namespace SniffTheWay;

export class UITextShadow
{
public:
	void Init(
		dh::RenderContext const & render_context,
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		Camera2d const & camera2d,
		std::string_view text,
		FontAtlas const & font_atlas,
		float font_size,
		glm::vec2 origin,
		UILabel::Align align);

	void RenderOffscreenTexture() const;
	void SetText(std::string_view text);
	void SetFontSize(float font_size);

private:
	MeshId<TextureVertex2d> create_fullscreen_quad(AssetManager & asset_manager) const;

private:
	AssetManager * m_asset_manager = nullptr;
	SceneRenderer * m_renderer = nullptr;

	// OpenGL renders framebuffer attachments with a bottom-left origin. The offscreen
	// text mask is later sampled as a texture in top-left UI space, so this pass needs
	// the opposite Y projection from the main screen camera to avoid an inverted mask.
	Camera2d m_offscreen_camera2d{ true /*flip_screen_y*/ };

	AssetId m_mask_tex_id;
	AssetId m_blur_temp_tex_id;
	AssetId m_blur_tex_id;

	std::unique_ptr<UILabel> m_mask_label;
	TextMaskPipeline::ObjectData m_mask_data;
	MeshId<TextureVertex2d> m_quad_mesh_id;

	BlurPipeline::ObjectData m_horizontal_blur_data;
	BlurPipeline::ObjectData m_vertical_blur_data;
	ShadowCompositePipeline::ObjectData m_composite_data;
	RenderObject m_mask_ro;
	RenderObject m_horizontal_blur_ro;
	RenderObject m_vertical_blur_ro;
	AssetId m_composite_ro_id;
};

void UITextShadow::Init(
	dh::RenderContext const & render_context,
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	Camera2d const & camera2d,
	std::string_view text,
	FontAtlas const & font_atlas,
	float font_size,
	glm::vec2 origin,
	UILabel::Align align)
{
	m_asset_manager = &asset_manager;
	m_renderer = &renderer;
	m_offscreen_camera2d.Init(0.0f /*left*/, UIWidth /*right*/, 0.0f /*top*/, UIHeight /*bottom*/);

	const auto text_mask_pipeline_id = asset_manager.AddPipeline<TextMaskPipeline>(m_offscreen_camera2d, asset_manager);
	const auto blur_pipeline_id = asset_manager.AddPipeline<BlurPipeline>(m_offscreen_camera2d, asset_manager);
	const auto shadow_composite_pipeline_id = asset_manager.AddPipeline<ShadowCompositePipeline>(camera2d, asset_manager);

	m_mask_tex_id = asset_manager.AddRenderTexture(static_cast<std::uint32_t>(UIWidth), static_cast<std::uint32_t>(UIHeight));
	m_blur_temp_tex_id = asset_manager.AddRenderTexture(static_cast<std::uint32_t>(UIWidth), static_cast<std::uint32_t>(UIHeight));
	m_blur_tex_id = asset_manager.AddRenderTexture(static_cast<std::uint32_t>(UIWidth), static_cast<std::uint32_t>(UIHeight));

	m_mask_label = std::make_unique<UILabel>(
		asset_manager,
		text,
		font_atlas,
		font_size,
		origin,
		align,
		glm::vec4{ 1.0f });
	TextPipeline::ObjectData const & label_data = m_mask_label->GetPipelineData();
	m_mask_data = TextMaskPipeline::ObjectData{
		.screen_px_range = label_data.screen_px_range,
		.bg_color = label_data.bg_color,
		.text_color = label_data.text_color,
		.tex_id = label_data.tex_id,
	};
	m_mask_ro = RenderObject("story text mask", m_mask_label->GetMeshId(), text_mask_pipeline_id);
	m_mask_ro.SetObjectData(&m_mask_data);

	m_quad_mesh_id = create_fullscreen_quad(asset_manager);

	m_horizontal_blur_data = BlurPipeline::ObjectData{
		.tex_id = m_mask_tex_id,
		.texel_step = glm::vec2{ 2.0f / UIWidth, 0.0f }
	};
	m_horizontal_blur_ro = RenderObject("story shadow horizontal blur", m_quad_mesh_id, blur_pipeline_id);
	m_horizontal_blur_ro.SetObjectData(&m_horizontal_blur_data);

	m_vertical_blur_data = BlurPipeline::ObjectData{
		.tex_id = m_blur_temp_tex_id,
		.texel_step = glm::vec2{ 0.0f, 2.0f / UIHeight }
	};
	m_vertical_blur_ro = RenderObject("story shadow vertical blur", m_quad_mesh_id, blur_pipeline_id);
	m_vertical_blur_ro.SetObjectData(&m_vertical_blur_data);

	m_composite_data = ShadowCompositePipeline::ObjectData{
		.tex_id = m_blur_tex_id,
		.color = glm::vec4{ 0.08f, 0.06f, 0.035f, 1.0f }
	};
	m_composite_ro_id = renderer.CreateRenderObject(
		"story shadow composite",
		m_quad_mesh_id,
		shadow_composite_pipeline_id,
		m_composite_data);
}

void UITextShadow::RenderOffscreenTexture() const
{
	if (!m_asset_manager || !m_renderer)
		return;

	dh::Texture const * mask_tex = m_asset_manager->GetTexture(m_mask_tex_id);
	dh::Texture const * blur_temp_tex = m_asset_manager->GetTexture(m_blur_temp_tex_id);
	dh::Texture const * blur_tex = m_asset_manager->GetTexture(m_blur_tex_id);
	if (!mask_tex || !blur_temp_tex || !blur_tex)
		return;

	m_renderer->RenderToTexture(*mask_tex, std::span<RenderObject const>{ &m_mask_ro, 1 }, glm::vec4{ 0.0f });

	m_renderer->RenderToTexture(*blur_temp_tex, std::span<RenderObject const>{ &m_horizontal_blur_ro, 1 }, glm::vec4{ 0.0f });

	m_renderer->RenderToTexture(*blur_tex, std::span<RenderObject const>{ &m_vertical_blur_ro, 1 }, glm::vec4{ 0.0f });
}

void UITextShadow::SetText(std::string_view text)
{
	if (m_mask_label)
		m_mask_label->SetText(text);
}

void UITextShadow::SetFontSize(float font_size)
{
	if (m_mask_label)
	{
		m_mask_label->SetFontSize(font_size);
		TextPipeline::ObjectData const & label_data = m_mask_label->GetPipelineData();
		m_mask_data.screen_px_range = label_data.screen_px_range;
	}
}

MeshId<TextureVertex2d> UITextShadow::create_fullscreen_quad(AssetManager & asset_manager) const
{
	std::vector<TextureVertex2d> verts{
		{ { 0.0f, 0.0f }, { 0.0f, 0.0f } },
		{ { UIWidth, 0.0f }, { 1.0f, 0.0f } },
		{ { 0.0f, UIHeight }, { 0.0f, 1.0f } },
		{ { UIWidth, UIHeight }, { 1.0f, 1.0f } },
	};

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3
	};

	return asset_manager.AddMesh(verts, indices);
}
