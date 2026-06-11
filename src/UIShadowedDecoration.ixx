// UIShadowedDecoration.ixx

module;

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>
#include <utility>

#include <glm/glm.hpp>

export module UIShadowedDecoration;

import AssetManager;
import AssetPool;
import Camera;
import DecorationAtlas;
import RenderObject;
import SceneRenderer;
import SniffTheWayConstants;
import Texture2dPipeline;
import TextureMaskPipeline;
import UIDecoration;
import UIElementShadowRenderer;

export class UIShadowedDecoration
{
public:
	void Init(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		Camera2d const & camera2d,
		std::string_view name,
		AssetId texture_id,
		SniffTheWay::Decorations::DecorationId decoration_id,
		glm::vec2 center,
		float scale,
		glm::vec4 color);

	void RenderOffscreenTexture() const;
	void SetDecorationId(SniffTheWay::Decorations::DecorationId decoration_id);
	void SetCenter(glm::vec2 center);
	void SetScale(float scale);
	void SetColor(glm::vec4 color);
	void SetOpacity(float opacity);
	void SetVisible(bool visible);

	UIDecoration const & GetUIDecoration() const { return m_decoration; }

private:
	using ShadowStyle = UIElementShadowRenderer::Style;

	static ShadowStyle create_shadow_style(float height);

	void update_layout();

private:
	AssetManager * m_asset_manager = nullptr;
	SceneRenderer * m_renderer = nullptr;

	UIDecoration m_decoration;
	UIDecoration m_mask_decoration;
	UIElementShadowRenderer m_shadow_renderer;

	std::string m_name;
	AssetId m_texture_id;
	SniffTheWay::Decorations::DecorationId m_decoration_id = SniffTheWay::Decorations::DecorationId::HorizontalDividerPawFlourish;
	glm::vec2 m_center{ 0.0f };
	float m_scale = 1.0f;
	glm::vec4 m_color{ 1.0f };
	float m_opacity = 1.0f;
	ShadowStyle m_shadow_style;

	TextureMaskPipeline::ObjectData m_mask_data;
	AssetId m_decoration_ro_id;
};

void UIShadowedDecoration::Init(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	Camera2d const & camera2d,
	std::string_view name,
	AssetId texture_id,
	SniffTheWay::Decorations::DecorationId decoration_id,
	glm::vec2 center,
	float scale,
	glm::vec4 color)
{
	m_asset_manager = &asset_manager;
	m_renderer = &renderer;
	m_name = std::string{ name };
	m_texture_id = texture_id;
	m_decoration_id = decoration_id;
	m_center = center;
	m_scale = scale;
	m_color = color;

	const auto texture_pipeline_id = asset_manager.AddPipeline<Texture2dPipeline>(camera2d, asset_manager);
	const auto texture_mask_pipeline_id = asset_manager.AddPipeline<TextureMaskPipeline>(
		m_shadow_renderer.GetOffscreenCamera(),
		asset_manager);

	m_decoration.Init(
		asset_manager,
		texture_id,
		decoration_id,
		center,
		scale,
		color);
	m_mask_decoration.Init(
		asset_manager,
		texture_id,
		decoration_id,
		glm::vec2{ 0.0f },
		scale);

	m_mask_data = TextureMaskPipeline::ObjectData{
		.tex_id = texture_id,
	};
	RenderObject mask_ro(m_name + " shadow mask", m_mask_decoration.GetMeshId(), texture_mask_pipeline_id);
	mask_ro.SetObjectData(&m_mask_data);

	m_shadow_style = create_shadow_style(m_decoration.GetBounds().Size().y);
	m_shadow_renderer.Init(
		asset_manager,
		renderer,
		camera2d,
		m_name,
		std::move(mask_ro),
		m_shadow_style);

	m_decoration_ro_id = renderer.CreateRenderObject(
		m_name + " decoration",
		SniffTheWay::RenderLayer::UIForeground,
		m_decoration.GetMeshId(),
		texture_pipeline_id,
		m_decoration.GetPipelineData());
	m_decoration.SetROId(m_decoration_ro_id);

	update_layout();
}

UIShadowedDecoration::ShadowStyle UIShadowedDecoration::create_shadow_style(float height)
{
	return ShadowStyle{
		.blur_radius = static_cast<int>(std::max(4.0f, height / 2.0f)),
		.alpha_boost = 1.5f,
		.sharp_blur_radius = 6,
		.sharp_alpha_boost = 2.0f,
		.offset = glm::vec2{ 0.0f },
		.color = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
	};
}

void UIShadowedDecoration::RenderOffscreenTexture() const
{
	m_shadow_renderer.RenderOffscreenTexture();
}

void UIShadowedDecoration::SetDecorationId(SniffTheWay::Decorations::DecorationId decoration_id)
{
	if (m_decoration_id == decoration_id)
		return;

	m_decoration_id = decoration_id;
	m_decoration.SetDecorationId(decoration_id);
	m_mask_decoration.SetDecorationId(decoration_id);

	m_shadow_style = create_shadow_style(m_decoration.GetBounds().Size().y);
	m_shadow_renderer.SetStyle(m_shadow_style);
	update_layout();
}

void UIShadowedDecoration::SetCenter(glm::vec2 center)
{
	if (m_center == center)
		return;

	m_center = center;
	m_decoration.SetCenter(center);
	update_layout();
}

void UIShadowedDecoration::SetScale(float scale)
{
	if (m_scale == scale)
		return;

	m_scale = scale;
	m_decoration.SetScale(scale);
	m_mask_decoration.SetScale(scale);

	m_shadow_style = create_shadow_style(m_decoration.GetBounds().Size().y);
	m_shadow_renderer.SetStyle(m_shadow_style);
	update_layout();
}

void UIShadowedDecoration::SetColor(glm::vec4 color)
{
	m_color = color;
	m_decoration.SetColor(glm::vec4{ color.r, color.g, color.b, color.a * m_opacity });
}

void UIShadowedDecoration::SetOpacity(float opacity)
{
	m_opacity = std::clamp(opacity, 0.0f, 1.0f);
	m_decoration.SetColor(glm::vec4{ m_color.r, m_color.g, m_color.b, m_color.a * m_opacity });
	m_shadow_renderer.SetOpacity(m_opacity);
}

void UIShadowedDecoration::SetVisible(bool visible)
{
	if (!m_renderer)
		return;

	m_shadow_renderer.SetVisible(visible);
	m_renderer->Show(m_decoration_ro_id, visible);
}

void UIShadowedDecoration::update_layout()
{
	if (!m_asset_manager || !m_renderer)
		return;

	UIDecoration::Bounds const & bounds = m_decoration.GetBounds();
	const UIElementShadowRenderer::Bounds shadow_bounds{
		.min = bounds.min,
		.max = bounds.max,
		.is_valid = bounds.is_valid
	};
	if (!bounds.is_valid)
	{
		m_shadow_renderer.SetBounds(shadow_bounds, 0.0f);
		return;
	}

	const float padding = m_shadow_renderer.GetMaskPadding();
	m_mask_decoration.SetCenter(bounds.Size() * 0.5f + glm::vec2{ padding });
	m_shadow_renderer.SetBounds(shadow_bounds, padding);
}
