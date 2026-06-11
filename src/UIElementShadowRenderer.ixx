// UIElementShadowRenderer.ixx

module;

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

export module UIElementShadowRenderer;

import Dreamhearth;

import AssetManager;
import AssetPool;
import BlurPipeline;
import Camera;
import RenderObject;
import SceneRenderer;
import ShadowBackdropPipeline;
import ShadowCompositePipeline;
import SniffTheWayConstants;
import Vertex;

using namespace SniffTheWay;

namespace dh = Dreamhearth;

export class UIElementShadowRenderer
{
public:
	struct Bounds
	{
		glm::vec2 min{ 0.0f };
		glm::vec2 max{ 0.0f };
		bool is_valid = false;

		glm::vec2 Size() const { return is_valid ? max - min : glm::vec2{ 0.0f }; }
	};

	struct Style
	{
		int blur_radius = 12;
		float alpha_boost = 1.5f;
		int sharp_blur_radius = 2;
		float sharp_alpha_boost = 1.5f;
		glm::vec2 offset{ 0.0f };
		glm::vec4 color{ 0.0f, 0.0f, 0.0f, 1.0f };
	};

	void Init(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		Camera2d const & camera2d,
		std::string_view name,
		RenderObject const & mask_ro,
		Style style);

	void RenderOffscreenTexture() const;
	void SetBounds(Bounds const & bounds, float backdrop_padding);
	void SetStyle(Style const & style);
	void SetOpacity(float opacity);
	void SetVisible(bool visible);
	void MarkDirty() { m_shadow_texture_dirty = true; }

	float GetMaskPadding() const;
	Camera2d const & GetOffscreenCamera() const { return m_offscreen_camera2d; }

private:
	void update_render_textures(std::uint32_t width, std::uint32_t height);
	void update_quad_meshes();

	MeshId<TextureVertex2d> create_shadow_backdrop_mesh(
		AssetManager & asset_manager,
		Bounds const & bounds,
		float padding) const;

	void update_shadow_backdrop_mesh(
		Bounds const & bounds,
		float padding);

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
	RenderObject const * m_mask_ro = nullptr;

	Camera2d m_offscreen_camera2d{ true /*flip_screen_y*/ };

	std::string m_name;
	Style m_style;
	float m_opacity = 1.0f;

	AssetId m_mask_tex_id;
	AssetId m_blur_temp_tex_id;
	AssetId m_blur_tex_id;
	AssetId m_sharp_blur_temp_tex_id;
	AssetId m_sharp_blur_tex_id;
	std::uint32_t m_texture_width = 1;
	std::uint32_t m_texture_height = 1;
	MeshId<TextureVertex2d> m_shadow_backdrop_mesh_id;

	BlurPipeline::ObjectData m_horizontal_blur_data;
	BlurPipeline::ObjectData m_vertical_blur_data;
	BlurPipeline::ObjectData m_sharp_horizontal_blur_data;
	BlurPipeline::ObjectData m_sharp_vertical_blur_data;
	ShadowBackdropPipeline::ObjectData m_shadow_backdrop_data;
	ShadowCompositePipeline::ObjectData m_composite_data;
	ShadowCompositePipeline::ObjectData m_sharp_composite_data;

	MeshId<TextureVertex2d> m_offscreen_quad_mesh_id;
	MeshId<TextureVertex2d> m_composite_quad_mesh_id;

	RenderObject m_horizontal_blur_ro;
	RenderObject m_vertical_blur_ro;
	RenderObject m_sharp_horizontal_blur_ro;
	RenderObject m_sharp_vertical_blur_ro;
	AssetId m_shadow_backdrop_ro_id;
	AssetId m_composite_ro_id;
	AssetId m_sharp_composite_ro_id;

	mutable bool m_shadow_texture_dirty = true;
};

void UIElementShadowRenderer::Init(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	Camera2d const & camera2d,
	std::string_view name,
	RenderObject const & mask_ro,
	Style style)
{
	m_asset_manager = &asset_manager;
	m_renderer = &renderer;
	m_mask_ro = &mask_ro;
	m_name = std::string{ name };
	m_style = style;

	const auto shadow_backdrop_pipeline_id = asset_manager.AddPipeline<ShadowBackdropPipeline>(camera2d);
	const auto blur_pipeline_id = asset_manager.AddPipeline<BlurPipeline>(m_offscreen_camera2d, asset_manager);
	const auto shadow_composite_pipeline_id = asset_manager.AddPipeline<ShadowCompositePipeline>(camera2d, asset_manager);

	m_mask_tex_id = asset_manager.AddRenderTexture(m_texture_width, m_texture_height);
	m_blur_temp_tex_id = asset_manager.AddRenderTexture(m_texture_width, m_texture_height);
	m_blur_tex_id = asset_manager.AddRenderTexture(m_texture_width, m_texture_height);
	m_sharp_blur_temp_tex_id = asset_manager.AddRenderTexture(m_texture_width, m_texture_height);
	m_sharp_blur_tex_id = asset_manager.AddRenderTexture(m_texture_width, m_texture_height);

	m_offscreen_quad_mesh_id = create_quad(asset_manager, glm::vec2{ 0.0f }, glm::vec2{ 1.0f });
	m_composite_quad_mesh_id = create_quad(asset_manager, glm::vec2{ 0.0f }, glm::vec2{ 1.0f });
	m_shadow_backdrop_mesh_id = create_shadow_backdrop_mesh(asset_manager, Bounds{}, 0.0f);

	m_horizontal_blur_data = BlurPipeline::ObjectData{
		.tex_id = m_mask_tex_id,
		.texel_step = glm::vec2{ 1.0f, 0.0f },
		.blur_radius = m_style.blur_radius,
		.alpha_boost = m_style.alpha_boost
	};
	m_horizontal_blur_ro = RenderObject(m_name + " shadow horizontal blur", m_offscreen_quad_mesh_id, blur_pipeline_id);
	m_horizontal_blur_ro.SetObjectData(&m_horizontal_blur_data);

	m_vertical_blur_data = BlurPipeline::ObjectData{
		.tex_id = m_blur_temp_tex_id,
		.texel_step = glm::vec2{ 0.0f, 1.0f },
		.blur_radius = m_style.blur_radius,
		.alpha_boost = m_style.alpha_boost
	};
	m_vertical_blur_ro = RenderObject(m_name + " shadow vertical blur", m_offscreen_quad_mesh_id, blur_pipeline_id);
	m_vertical_blur_ro.SetObjectData(&m_vertical_blur_data);

	m_sharp_horizontal_blur_data = BlurPipeline::ObjectData{
		.tex_id = m_mask_tex_id,
		.texel_step = glm::vec2{ 1.0f, 0.0f },
		.blur_radius = m_style.sharp_blur_radius,
		.alpha_boost = m_style.sharp_alpha_boost
	};
	m_sharp_horizontal_blur_ro = RenderObject(m_name + " sharp shadow horizontal blur", m_offscreen_quad_mesh_id, blur_pipeline_id);
	m_sharp_horizontal_blur_ro.SetObjectData(&m_sharp_horizontal_blur_data);

	m_sharp_vertical_blur_data = BlurPipeline::ObjectData{
		.tex_id = m_sharp_blur_temp_tex_id,
		.texel_step = glm::vec2{ 0.0f, 1.0f },
		.blur_radius = m_style.sharp_blur_radius,
		.alpha_boost = m_style.sharp_alpha_boost
	};
	m_sharp_vertical_blur_ro = RenderObject(m_name + " sharp shadow vertical blur", m_offscreen_quad_mesh_id, blur_pipeline_id);
	m_sharp_vertical_blur_ro.SetObjectData(&m_sharp_vertical_blur_data);

	m_shadow_backdrop_ro_id = renderer.CreateRenderObject(
		m_name + " shadow backdrop",
		RenderLayer::UIShadow,
		m_shadow_backdrop_mesh_id,
		shadow_backdrop_pipeline_id,
		m_shadow_backdrop_data);

	m_composite_data = ShadowCompositePipeline::ObjectData{
		.tex_id = m_blur_tex_id,
		.color = m_style.color,
	};
	m_composite_ro_id = renderer.CreateRenderObject(
		m_name + " shadow composite",
		RenderLayer::UIShadow,
		m_composite_quad_mesh_id,
		shadow_composite_pipeline_id,
		m_composite_data);

	m_sharp_composite_data = ShadowCompositePipeline::ObjectData{
		.tex_id = m_sharp_blur_tex_id,
		.color = m_style.color,
	};
	m_sharp_composite_ro_id = renderer.CreateRenderObject(
		m_name + " sharp shadow composite",
		RenderLayer::UIShadow,
		m_composite_quad_mesh_id,
		shadow_composite_pipeline_id,
		m_sharp_composite_data);
}

void UIElementShadowRenderer::RenderOffscreenTexture() const
{
	if (!m_shadow_texture_dirty || !m_asset_manager || !m_renderer || !m_mask_ro)
		return;

	dh::Texture const * mask_tex = m_asset_manager->GetTexture(m_mask_tex_id);
	dh::Texture const * blur_temp_tex = m_asset_manager->GetTexture(m_blur_temp_tex_id);
	dh::Texture const * blur_tex = m_asset_manager->GetTexture(m_blur_tex_id);
	dh::Texture const * sharp_blur_temp_tex = m_asset_manager->GetTexture(m_sharp_blur_temp_tex_id);
	dh::Texture const * sharp_blur_tex = m_asset_manager->GetTexture(m_sharp_blur_tex_id);
	if (!mask_tex || !blur_temp_tex || !blur_tex || !sharp_blur_temp_tex || !sharp_blur_tex)
		return;

	m_renderer->RenderToTexture(*mask_tex, std::span<RenderObject const>{ m_mask_ro, 1 }, glm::vec4{ 0.0f });
	m_renderer->RenderToTexture(*blur_temp_tex, std::span<RenderObject const>{ &m_horizontal_blur_ro, 1 }, glm::vec4{ 0.0f });
	m_renderer->RenderToTexture(*blur_tex, std::span<RenderObject const>{ &m_vertical_blur_ro, 1 }, glm::vec4{ 0.0f });
	m_renderer->RenderToTexture(*sharp_blur_temp_tex, std::span<RenderObject const>{ &m_sharp_horizontal_blur_ro, 1 }, glm::vec4{ 0.0f });
	m_renderer->RenderToTexture(*sharp_blur_tex, std::span<RenderObject const>{ &m_sharp_vertical_blur_ro, 1 }, glm::vec4{ 0.0f });

	m_shadow_texture_dirty = false;
}

void UIElementShadowRenderer::SetBounds(Bounds const & bounds, float backdrop_padding)
{
	if (!m_asset_manager || !m_renderer)
		return;

	if (!bounds.is_valid)
	{
		update_shadow_backdrop_mesh(bounds, backdrop_padding);
		m_renderer->Show(m_shadow_backdrop_ro_id, false);
		m_renderer->Show(m_composite_ro_id, false);
		m_renderer->Show(m_sharp_composite_ro_id, false);
		m_shadow_texture_dirty = true;
		return;
	}

	m_renderer->Show(m_shadow_backdrop_ro_id, true);
	m_renderer->Show(m_composite_ro_id, true);
	m_renderer->Show(m_sharp_composite_ro_id, true);
	update_shadow_backdrop_mesh(bounds, backdrop_padding);

	const float padding = GetMaskPadding();
	const glm::vec2 size = bounds.Size();
	const auto width = static_cast<std::uint32_t>(std::max(1.0f, std::ceil(size.x + padding * 2.0f)));
	const auto height = static_cast<std::uint32_t>(std::max(1.0f, std::ceil(size.y + padding * 2.0f)));

	update_render_textures(width, height);

	const glm::vec2 composite_top_left = bounds.min + m_style.offset - glm::vec2{ padding };
	update_quad(m_composite_quad_mesh_id, composite_top_left, glm::vec2{ static_cast<float>(width), static_cast<float>(height) });
	update_quad_meshes();

	m_shadow_texture_dirty = true;
}

void UIElementShadowRenderer::SetStyle(Style const & style)
{
	m_style = style;
	m_horizontal_blur_data.blur_radius = m_style.blur_radius;
	m_horizontal_blur_data.alpha_boost = m_style.alpha_boost;
	m_vertical_blur_data.blur_radius = m_style.blur_radius;
	m_vertical_blur_data.alpha_boost = m_style.alpha_boost;
	m_sharp_horizontal_blur_data.blur_radius = m_style.sharp_blur_radius;
	m_sharp_horizontal_blur_data.alpha_boost = m_style.sharp_alpha_boost;
	m_sharp_vertical_blur_data.blur_radius = m_style.sharp_blur_radius;
	m_sharp_vertical_blur_data.alpha_boost = m_style.sharp_alpha_boost;
	SetOpacity(m_opacity);
	m_shadow_texture_dirty = true;
}

void UIElementShadowRenderer::SetOpacity(float opacity)
{
	m_opacity = std::clamp(opacity, 0.0f, 1.0f);
	m_composite_data.color = glm::vec4{
		m_style.color.r,
		m_style.color.g,
		m_style.color.b,
		m_style.color.a * m_opacity
	};
	m_sharp_composite_data.color = m_composite_data.color;
	m_shadow_backdrop_data.color.a = 0.6f * m_opacity;
}

void UIElementShadowRenderer::SetVisible(bool visible)
{
	if (!m_renderer)
		return;

	m_renderer->Show(m_shadow_backdrop_ro_id, visible);
	m_renderer->Show(m_composite_ro_id, visible);
	m_renderer->Show(m_sharp_composite_ro_id, visible);
}

float UIElementShadowRenderer::GetMaskPadding() const
{
	const int blur_radius = std::max(0, m_style.blur_radius);
	return static_cast<float>(std::max(2, blur_radius * 2));
}

void UIElementShadowRenderer::update_render_textures(std::uint32_t width, std::uint32_t height)
{
	if (!m_asset_manager || (m_texture_width == width && m_texture_height == height))
		return;

	if (m_mask_tex_id.IsValid())
		m_asset_manager->RemoveTexture(m_mask_tex_id);
	if (m_blur_temp_tex_id.IsValid())
		m_asset_manager->RemoveTexture(m_blur_temp_tex_id);
	if (m_blur_tex_id.IsValid())
		m_asset_manager->RemoveTexture(m_blur_tex_id);
	if (m_sharp_blur_temp_tex_id.IsValid())
		m_asset_manager->RemoveTexture(m_sharp_blur_temp_tex_id);
	if (m_sharp_blur_tex_id.IsValid())
		m_asset_manager->RemoveTexture(m_sharp_blur_tex_id);

	m_texture_width = width;
	m_texture_height = height;
	m_mask_tex_id = m_asset_manager->AddRenderTexture(width, height);
	m_blur_temp_tex_id = m_asset_manager->AddRenderTexture(width, height);
	m_blur_tex_id = m_asset_manager->AddRenderTexture(width, height);
	m_sharp_blur_temp_tex_id = m_asset_manager->AddRenderTexture(width, height);
	m_sharp_blur_tex_id = m_asset_manager->AddRenderTexture(width, height);

	m_horizontal_blur_data.tex_id = m_mask_tex_id;
	m_vertical_blur_data.tex_id = m_blur_temp_tex_id;
	m_composite_data.tex_id = m_blur_tex_id;
	m_sharp_horizontal_blur_data.tex_id = m_mask_tex_id;
	m_sharp_vertical_blur_data.tex_id = m_sharp_blur_temp_tex_id;
	m_sharp_composite_data.tex_id = m_sharp_blur_tex_id;
}

void UIElementShadowRenderer::update_quad_meshes()
{
	update_quad(
		m_offscreen_quad_mesh_id,
		glm::vec2{ 0.0f },
		glm::vec2{ static_cast<float>(m_texture_width), static_cast<float>(m_texture_height) });
	m_offscreen_camera2d.Init(0.0f, static_cast<float>(m_texture_width), 0.0f, static_cast<float>(m_texture_height));

	m_horizontal_blur_data.texel_step = glm::vec2{ 1.0f / static_cast<float>(m_texture_width), 0.0f };
	m_vertical_blur_data.texel_step = glm::vec2{ 0.0f, 1.0f / static_cast<float>(m_texture_height) };
	m_sharp_horizontal_blur_data.texel_step = glm::vec2{ 1.0f / static_cast<float>(m_texture_width), 0.0f };
	m_sharp_vertical_blur_data.texel_step = glm::vec2{ 0.0f, 1.0f / static_cast<float>(m_texture_height) };
}

MeshId<TextureVertex2d> UIElementShadowRenderer::create_shadow_backdrop_mesh(
	AssetManager & asset_manager,
	Bounds const & bounds,
	float padding) const
{
	if (!bounds.is_valid)
		return create_quad(asset_manager, glm::vec2{ 0.0f }, glm::vec2{ 1.0f });

	glm::vec2 const outer_min = bounds.min - glm::vec2{ padding };
	glm::vec2 const outer_size = bounds.Size() + glm::vec2{ padding * 2.0f };
	return create_quad(asset_manager, outer_min, outer_size);
}

void UIElementShadowRenderer::update_shadow_backdrop_mesh(
	Bounds const & bounds,
	float padding)
{
	if (!m_asset_manager)
		return;

	if (!bounds.is_valid)
	{
		update_quad(m_shadow_backdrop_mesh_id, glm::vec2{ 0.0f }, glm::vec2{ 1.0f });
		m_shadow_backdrop_data = ShadowBackdropPipeline::ObjectData{
			.inner_min_uv = glm::vec2{ 0.5f },
			.inner_max_uv = glm::vec2{ 0.5f },
			.color = glm::vec4{ 0.0f },
		};
		return;
	}

	glm::vec2 const outer_min = bounds.min - glm::vec2{ padding };
	glm::vec2 const outer_size = bounds.Size() + glm::vec2{ padding * 2.0f };
	update_quad(m_shadow_backdrop_mesh_id, outer_min, outer_size);

	glm::vec2 const inner_min_uv = glm::vec2{ padding } / outer_size;
	m_shadow_backdrop_data = ShadowBackdropPipeline::ObjectData{
		.inner_min_uv = inner_min_uv,
		.inner_max_uv = glm::vec2{ 1.0f } - inner_min_uv,
		.color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.6f * m_opacity },
	};
}

MeshId<TextureVertex2d> UIElementShadowRenderer::create_quad(
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

void UIElementShadowRenderer::update_quad(
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
