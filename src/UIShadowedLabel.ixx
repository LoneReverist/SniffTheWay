// UIShadowedLabel.ixx

module;

#include <algorithm>
#include <string>
#include <string_view>

#include <glm/glm.hpp>

export module UIShadowedLabel;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Camera;
import FontAtlas;
import RenderObject;
import SceneRenderer;
import TextMaskPipeline;
import TextPipeline;
import UIElementShadowRenderer;
import UILabel;
import Vertex;

namespace dh = Dreamhearth;

export class UIShadowedLabel
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
	void SetTextColor(glm::vec4 text_color);
	void SetOpacity(float opacity);
	void SetOrigin(glm::vec2 origin);
	void SetAlign(UILabel::Align align);
	void SetVisible(bool visible);

	UILabel const & GetUILabel() const { return m_label; }

private:
	using ShadowStyle = UIElementShadowRenderer::Style;

	static ShadowStyle create_shadow_style(float font_size);

	void update_layout();

private:
	AssetManager * m_asset_manager = nullptr;
	SceneRenderer * m_renderer = nullptr;

	UILabel m_label;
	UILabel m_mask_label;
	UIElementShadowRenderer m_shadow_renderer;

	std::string m_name;
	glm::vec2 m_origin{ 0.0f };
	UILabel::Align m_align = UILabel::Align::Left;
	glm::vec4 m_text_color{ 1.0f };
	float m_opacity = 1.0f;
	float m_font_line_height_scale = 0.0f;
	float m_line_height = 0.0f;
	ShadowStyle m_shadow_style;

	TextMaskPipeline::ObjectData m_mask_data;
	RenderObject m_mask_ro;
	AssetId m_label_ro_id;
};

void UIShadowedLabel::Init(
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
	m_text_color = text_color;
	m_font_line_height_scale = font_atlas.GetLineHeight();
	m_line_height = m_font_line_height_scale * font_size;
	m_shadow_style = create_shadow_style(font_size);

	const auto text_pipeline_id = asset_manager.AddPipeline<TextPipeline>(camera2d, asset_manager);
	const auto text_mask_pipeline_id = asset_manager.AddPipeline<TextMaskPipeline>(
		m_shadow_renderer.GetOffscreenCamera(),
		asset_manager);

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

	m_shadow_renderer.Init(
		asset_manager,
		renderer,
		camera2d,
		m_name,
		m_mask_ro,
		m_shadow_style);

	m_label_ro_id = renderer.CreateUIRenderObject(
		m_name + " label",
		m_label.GetMeshId(),
		text_pipeline_id,
		m_label.GetPipelineData());
	m_label.SetROId(m_label_ro_id);

	update_layout();
}

UIShadowedLabel::ShadowStyle UIShadowedLabel::create_shadow_style(float font_size)
{
//	constexpr float ReferenceSmallFontSize = 36.0f;
//	constexpr float ReferenceSmallAlphaBoost = 1.8f;
//	constexpr float ReferenceLargeFontSize = 64.0f;
//	constexpr float ReferenceLargeAlphaBoost = 1.5f;
//
//	constexpr float alpha_boost_slope =
//		(ReferenceLargeAlphaBoost - ReferenceSmallAlphaBoost) / (ReferenceLargeFontSize - ReferenceSmallFontSize);
//	float alpha_boost = ReferenceSmallAlphaBoost + alpha_boost_slope * (font_size - ReferenceSmallFontSize);

	return ShadowStyle{
		.blur_radius = static_cast<int>(font_size / 2.0f),
		.alpha_boost = 1.5f,
		.sharp_blur_radius = 6,
		.sharp_alpha_boost = 2.0f,
		.offset = glm::vec2{0.0f},
		.color = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
	};
}

void UIShadowedLabel::RenderOffscreenTexture() const
{
	m_shadow_renderer.RenderOffscreenTexture();
}

void UIShadowedLabel::SetText(std::string_view text)
{
	m_label.SetText(text);
	m_mask_label.SetText(text);
	update_layout();
}

void UIShadowedLabel::SetFontSize(float font_size)
{
	m_label.SetFontSize(font_size);
	m_mask_label.SetFontSize(font_size);
	m_line_height = m_font_line_height_scale * font_size;

	m_shadow_style = create_shadow_style(font_size);
	m_shadow_renderer.SetStyle(m_shadow_style);

	TextPipeline::ObjectData const & label_data = m_mask_label.GetPipelineData();
	m_mask_data.screen_px_range = label_data.screen_px_range;

	update_layout();
}

void UIShadowedLabel::SetTextColor(glm::vec4 text_color)
{
	m_text_color = text_color;
	m_label.SetTextColor(glm::vec4{ text_color.r, text_color.g, text_color.b, text_color.a * m_opacity });
}

void UIShadowedLabel::SetOpacity(float opacity)
{
	m_opacity = std::clamp(opacity, 0.0f, 1.0f);
	m_label.SetTextColor(glm::vec4{ m_text_color.r, m_text_color.g, m_text_color.b, m_text_color.a * m_opacity });
	m_shadow_renderer.SetOpacity(m_opacity);
}

void UIShadowedLabel::SetOrigin(glm::vec2 origin)
{
	if (m_origin == origin)
		return;

	m_origin = origin;
	m_label.SetOrigin(origin);
	update_layout();
}

void UIShadowedLabel::SetVisible(bool visible)
{
	if (!m_renderer)
		return;

	m_shadow_renderer.SetVisible(visible);
	m_renderer->Show(m_label_ro_id, visible);
}

void UIShadowedLabel::SetAlign(UILabel::Align align)
{
	if (m_align == align)
		return;

	m_align = align;
	m_label.SetAlign(align);
	m_mask_label.SetAlign(align);
	update_layout();
}

void UIShadowedLabel::update_layout()
{
	if (!m_asset_manager || !m_renderer)
		return;

	UILabel::Bounds const & bounds = m_label.GetBounds();
	const UIElementShadowRenderer::Bounds shadow_bounds{
		.min = bounds.min,
		.max = bounds.max,
		.is_valid = bounds.is_valid
	};
	if (!bounds.is_valid)
	{
		m_shadow_renderer.SetBounds(shadow_bounds, m_line_height * 2.0f);
		return;
	}

	const float padding = m_shadow_renderer.GetMaskPadding();
	const glm::vec2 mask_origin = m_origin - bounds.min + glm::vec2{ padding };
	m_mask_label.SetOrigin(mask_origin);
	m_shadow_renderer.SetBounds(shadow_bounds, m_line_height * 2.0f);
}

