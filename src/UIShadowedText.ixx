// UIShadowedText.ixx

module;

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

export module UIShadowedText;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import AssetPool;
import BlurPipeline;
import Camera;
import FontAtlas;
import RenderObject;
import SceneRenderer;
import ShadowCompositePipeline;
import TextMaskPipeline;
import TextPipeline;
import UILabel;
import Vertex;

export class UIShadowedText
{
public:
	void Init(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		Camera2d const & camera2d,
		std::string_view name,
		std::string_view text,
		FontAtlas const & font_atlas,
		float font_size,
		glm::vec2 origin,
		UILabel::Align align,
		glm::vec4 text_color);

	void RenderOffscreenTexture() const;
	void SetText(std::string_view text);
	void SetFontSize(float font_size);
	void SetOrigin(glm::vec2 origin);
	void SetAlign(UILabel::Align align);

	UILabel const & GetUILabel() const { return m_label; }

private:
	struct ShadowStyle
	{
		int blur_radius = 12;
		float alpha_boost = 1.5f;
		glm::vec2 offset{ 0.0f };
		glm::vec4 color{ 0.0f, 0.0f, 0.0f, 1.0f };
	};

	static ShadowStyle create_shadow_style(float font_size);

	void update_layout();
	void update_render_textures(std::uint32_t width, std::uint32_t height);
	void update_quad_meshes();

	MeshId<TextureVertex2d> create_quad(
		AssetManager & asset_manager,
		glm::vec2 top_left,
		glm::vec2 size) const;

	void update_quad(
		MeshId<TextureVertex2d> mesh_id,
		glm::vec2 top_left,
		glm::vec2 size) const;

private:
	AssetManager * m_asset_manager = nullptr;
	SceneRenderer * m_renderer = nullptr;

	Camera2d m_offscreen_camera2d{ true /*flip_screen_y*/ };

	UILabel m_label;
	UILabel m_mask_label;

	std::string m_name;
	glm::vec2 m_origin{ 0.0f };
	UILabel::Align m_align = UILabel::Align::Left;
	ShadowStyle m_shadow_style;

	AssetId m_mask_tex_id;
	AssetId m_blur_temp_tex_id;
	AssetId m_blur_tex_id;
	std::uint32_t m_texture_width = 1;
	std::uint32_t m_texture_height = 1;

	TextMaskPipeline::ObjectData m_mask_data;
	BlurPipeline::ObjectData m_horizontal_blur_data;
	BlurPipeline::ObjectData m_vertical_blur_data;
	ShadowCompositePipeline::ObjectData m_composite_data;

	MeshId<TextureVertex2d> m_offscreen_quad_mesh_id;
	MeshId<TextureVertex2d> m_composite_quad_mesh_id;

	RenderObject m_mask_ro;
	RenderObject m_horizontal_blur_ro;
	RenderObject m_vertical_blur_ro;
	AssetId m_composite_ro_id;
	AssetId m_label_ro_id;

	mutable bool m_shadow_texture_dirty = true;
};

void UIShadowedText::Init(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	Camera2d const & camera2d,
	std::string_view name,
	std::string_view text,
	FontAtlas const & font_atlas,
	float font_size,
	glm::vec2 origin,
	UILabel::Align align,
	glm::vec4 text_color)
{
	m_asset_manager = &asset_manager;
	m_renderer = &renderer;
	m_name = std::string{ name };
	m_origin = origin;
	m_align = align;
	m_shadow_style = create_shadow_style(font_size);

	const auto text_pipeline_id = asset_manager.AddPipeline<TextPipeline>(camera2d, asset_manager);
	const auto text_mask_pipeline_id = asset_manager.AddPipeline<TextMaskPipeline>(m_offscreen_camera2d, asset_manager);
	const auto blur_pipeline_id = asset_manager.AddPipeline<BlurPipeline>(m_offscreen_camera2d, asset_manager);
	const auto shadow_composite_pipeline_id = asset_manager.AddPipeline<ShadowCompositePipeline>(camera2d, asset_manager);

	m_label.Init(
		asset_manager,
		text,
		font_atlas,
		font_size,
		origin,
		align,
		text_color);
	m_mask_label.Init(
		asset_manager,
		text,
		font_atlas,
		font_size,
		origin,
		align,
		glm::vec4{ 1.0f });

	TextPipeline::ObjectData const & label_data = m_mask_label.GetPipelineData();
	m_mask_data = TextMaskPipeline::ObjectData{
		.screen_px_range = label_data.screen_px_range,
		.bg_color = label_data.bg_color,
		.text_color = label_data.text_color,
		.tex_id = label_data.tex_id,
	};
	m_mask_ro = RenderObject(m_name + " shadow mask", m_mask_label.GetMeshId(), text_mask_pipeline_id);
	m_mask_ro.SetObjectData(&m_mask_data);

	m_mask_tex_id = asset_manager.AddRenderTexture(m_texture_width, m_texture_height);
	m_blur_temp_tex_id = asset_manager.AddRenderTexture(m_texture_width, m_texture_height);
	m_blur_tex_id = asset_manager.AddRenderTexture(m_texture_width, m_texture_height);

	m_offscreen_quad_mesh_id = create_quad(asset_manager, glm::vec2{ 0.0f }, glm::vec2{ 1.0f });
	m_composite_quad_mesh_id = create_quad(asset_manager, glm::vec2{ 0.0f }, glm::vec2{ 1.0f });

	m_horizontal_blur_data = BlurPipeline::ObjectData{
		.tex_id = m_mask_tex_id,
		.texel_step = glm::vec2{ 1.0f, 0.0f },
		.blur_radius = m_shadow_style.blur_radius,
		.alpha_boost = m_shadow_style.alpha_boost
	};
	m_horizontal_blur_ro = RenderObject(m_name + " shadow horizontal blur", m_offscreen_quad_mesh_id, blur_pipeline_id);
	m_horizontal_blur_ro.SetObjectData(&m_horizontal_blur_data);

	m_vertical_blur_data = BlurPipeline::ObjectData{
		.tex_id = m_blur_temp_tex_id,
		.texel_step = glm::vec2{ 0.0f, 1.0f },
		.blur_radius = m_shadow_style.blur_radius,
		.alpha_boost = m_shadow_style.alpha_boost
	};
	m_vertical_blur_ro = RenderObject(m_name + " shadow vertical blur", m_offscreen_quad_mesh_id, blur_pipeline_id);
	m_vertical_blur_ro.SetObjectData(&m_vertical_blur_data);

	m_composite_data = ShadowCompositePipeline::ObjectData{
		.tex_id = m_blur_tex_id,
		.color = m_shadow_style.color,
	};
	m_composite_ro_id = renderer.CreateUIRenderObject(
		m_name + " shadow composite",
		m_composite_quad_mesh_id,
		shadow_composite_pipeline_id,
		m_composite_data);

	m_label_ro_id = renderer.CreateUIRenderObject(
		m_name + " label",
		m_label.GetMeshId(),
		text_pipeline_id,
		m_label.GetPipelineData());
	m_label.SetROId(m_label_ro_id);

	update_layout();
}

UIShadowedText::ShadowStyle UIShadowedText::create_shadow_style(float font_size)
{
	constexpr float ReferenceSmallFontSize = 36.0f;
	constexpr float ReferenceSmallAlphaBoost = 1.8f;
	constexpr float ReferenceLargeFontSize = 64.0f;
	constexpr float ReferenceLargeAlphaBoost = 1.5f;

	constexpr float alpha_boost_slope =
		(ReferenceLargeAlphaBoost - ReferenceSmallAlphaBoost) / (ReferenceLargeFontSize - ReferenceSmallFontSize);

	return ShadowStyle{
		.blur_radius = static_cast<int>(font_size / 6.0f),
		.alpha_boost = ReferenceSmallAlphaBoost + alpha_boost_slope * (font_size - ReferenceSmallFontSize),
		.offset = glm::vec2{0.0f},
		.color = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
	};
}

void UIShadowedText::RenderOffscreenTexture() const
{
	if (!m_shadow_texture_dirty || !m_asset_manager || !m_renderer)
		return;

	dh::Texture const * mask_tex = m_asset_manager->GetTexture(m_mask_tex_id);
	dh::Texture const * blur_temp_tex = m_asset_manager->GetTexture(m_blur_temp_tex_id);
	dh::Texture const * blur_tex = m_asset_manager->GetTexture(m_blur_tex_id);
	if (!mask_tex || !blur_temp_tex || !blur_tex)
		return;

	m_renderer->RenderToTexture(*mask_tex, std::span<RenderObject const>{ &m_mask_ro, 1 }, glm::vec4{ 0.0f });
	m_renderer->RenderToTexture(*blur_temp_tex, std::span<RenderObject const>{ &m_horizontal_blur_ro, 1 }, glm::vec4{ 0.0f });
	m_renderer->RenderToTexture(*blur_tex, std::span<RenderObject const>{ &m_vertical_blur_ro, 1 }, glm::vec4{ 0.0f });

	m_shadow_texture_dirty = false;
}

void UIShadowedText::SetText(std::string_view text)
{
	m_label.SetText(text);
	m_mask_label.SetText(text);
	update_layout();
}

void UIShadowedText::SetFontSize(float font_size)
{
	m_label.SetFontSize(font_size);
	m_mask_label.SetFontSize(font_size);

	m_shadow_style = create_shadow_style(font_size);
	m_horizontal_blur_data.blur_radius = m_shadow_style.blur_radius;
	m_horizontal_blur_data.alpha_boost = m_shadow_style.alpha_boost;
	m_vertical_blur_data.blur_radius = m_shadow_style.blur_radius;
	m_vertical_blur_data.alpha_boost = m_shadow_style.alpha_boost;

	TextPipeline::ObjectData const & label_data = m_mask_label.GetPipelineData();
	m_mask_data.screen_px_range = label_data.screen_px_range;

	update_layout();
}

void UIShadowedText::SetOrigin(glm::vec2 origin)
{
	if (m_origin == origin)
		return;

	m_origin = origin;
	m_label.SetOrigin(origin);
	update_layout();
}

void UIShadowedText::SetAlign(UILabel::Align align)
{
	if (m_align == align)
		return;

	m_align = align;
	m_label.SetAlign(align);
	m_mask_label.SetAlign(align);
	update_layout();
}

void UIShadowedText::update_layout()
{
	if (!m_asset_manager || !m_renderer)
		return;

	UILabel::Bounds const & bounds = m_label.GetBounds();
	if (!bounds.is_valid)
	{
		m_renderer->Show(m_composite_ro_id, false);
		m_shadow_texture_dirty = true;
		return;
	}

	m_renderer->Show(m_composite_ro_id, true);

	const int blur_radius = std::max(0, m_shadow_style.blur_radius);
	const float padding = static_cast<float>(std::max(2, blur_radius * 2));
	const glm::vec2 text_size = bounds.Size();
	const auto width = static_cast<std::uint32_t>(std::max(1.0f, std::ceil(text_size.x + padding * 2.0f)));
	const auto height = static_cast<std::uint32_t>(std::max(1.0f, std::ceil(text_size.y + padding * 2.0f)));

	update_render_textures(width, height);

	const glm::vec2 mask_origin = m_origin - bounds.min + glm::vec2{ padding };
	m_mask_label.SetOrigin(mask_origin);

	const glm::vec2 composite_top_left = bounds.min + m_shadow_style.offset - glm::vec2{ padding };
	update_quad(m_composite_quad_mesh_id, composite_top_left, glm::vec2{ static_cast<float>(width), static_cast<float>(height) });
	update_quad_meshes();

	m_shadow_texture_dirty = true;
}

void UIShadowedText::update_render_textures(std::uint32_t width, std::uint32_t height)
{
	if (!m_asset_manager || (m_texture_width == width && m_texture_height == height))
		return;

	if (m_mask_tex_id.IsValid())
		m_asset_manager->RemoveTexture(m_mask_tex_id);
	if (m_blur_temp_tex_id.IsValid())
		m_asset_manager->RemoveTexture(m_blur_temp_tex_id);
	if (m_blur_tex_id.IsValid())
		m_asset_manager->RemoveTexture(m_blur_tex_id);

	m_texture_width = width;
	m_texture_height = height;
	m_mask_tex_id = m_asset_manager->AddRenderTexture(width, height);
	m_blur_temp_tex_id = m_asset_manager->AddRenderTexture(width, height);
	m_blur_tex_id = m_asset_manager->AddRenderTexture(width, height);

	m_horizontal_blur_data.tex_id = m_mask_tex_id;
	m_vertical_blur_data.tex_id = m_blur_temp_tex_id;
	m_composite_data.tex_id = m_blur_tex_id;
}

void UIShadowedText::update_quad_meshes()
{
	update_quad(
		m_offscreen_quad_mesh_id,
		glm::vec2{ 0.0f },
		glm::vec2{ static_cast<float>(m_texture_width), static_cast<float>(m_texture_height) });
	m_offscreen_camera2d.Init(0.0f, static_cast<float>(m_texture_width), 0.0f, static_cast<float>(m_texture_height));

	m_horizontal_blur_data.texel_step = glm::vec2{ 1.0f / static_cast<float>(m_texture_width), 0.0f };
	m_vertical_blur_data.texel_step = glm::vec2{ 0.0f, 1.0f / static_cast<float>(m_texture_height) };
}

MeshId<TextureVertex2d> UIShadowedText::create_quad(
	AssetManager & asset_manager,
	glm::vec2 top_left,
	glm::vec2 size) const
{
	std::vector<TextureVertex2d> verts{
		{ { top_left.x, top_left.y }, { 0.0f, 0.0f } },
		{ { top_left.x + size.x, top_left.y }, { 1.0f, 0.0f } },
		{ { top_left.x, top_left.y + size.y }, { 0.0f, 1.0f } },
		{ { top_left.x + size.x, top_left.y + size.y }, { 1.0f, 1.0f } },
	};

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3
	};

	return asset_manager.AddMesh(verts, indices);
}

void UIShadowedText::update_quad(
	MeshId<TextureVertex2d> mesh_id,
	glm::vec2 top_left,
	glm::vec2 size) const
{
	if (!m_asset_manager)
		return;

	std::vector<TextureVertex2d> verts{
		{ { top_left.x, top_left.y }, { 0.0f, 0.0f } },
		{ { top_left.x + size.x, top_left.y }, { 1.0f, 0.0f } },
		{ { top_left.x, top_left.y + size.y }, { 0.0f, 1.0f } },
		{ { top_left.x + size.x, top_left.y + size.y }, { 1.0f, 1.0f } },
	};

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3
	};

	m_asset_manager->UpdateMesh(mesh_id, verts, indices);
}
