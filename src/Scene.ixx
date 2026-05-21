// Scene.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module Scene;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import AssetPool;
import Baby;
import Background;
import BackgroundTexPipeline;
import Camera;
import Dog;
import EditorGrid;
import FontAtlas;
import Input;
import LinePipeline;
import Polygon2d;
import SceneRenderer;
import SpritePipeline;
import TextMesh;
import TextPipeline;
import Vertex;

export class Scene
{
public:
	explicit Scene(dh::RenderContext const & render_context, std::string const & title, float dpi_scale_factor);

	void OnViewportResized(int width, int height);
	void OnDPIScalingFactorChanged(float dpi_scale_factor);

	bool Update(float dt, Input const & input);
	void Render() const;

private:
	std::unique_ptr<TextMesh> create_text_mesh(
		std::string const & text,
		FontAtlas const & font_atlas,
		float font_size,
		glm::vec2 origin,
		int viewport_width,
		int viewport_height);

private:
	dh::RenderContext const & m_render_context;
	std::string const m_title;

	AssetManager m_asset_manager;
	SceneRenderer m_renderer;
	Camera3d m_camera3d;
	Camera2d m_camera2d;

	std::unique_ptr<FontAtlas> m_arial_font;

	std::unique_ptr<TextMesh> m_fps_mesh;
	std::unique_ptr<TextMesh> m_title_mesh;

	TextPipeline::ObjectData m_fps_label;
	TextPipeline::ObjectData m_title_label;

	Background m_background;
	Polygon2d m_bounds;
	EditorGrid m_grid;
	Dog m_dog;
	Baby m_baby;

	float m_frame_timer = 0.0f;
	int m_frame_count = 0;
};

std::unique_ptr<TextMesh> Scene::create_text_mesh(
	std::string const & text,
	FontAtlas const & font_atlas,
	float font_size,
	glm::vec2 origin,
	int viewport_width,
	int viewport_height)
{
	std::uint32_t font_tex_width = 0, font_tex_height = 0;
	dh::Texture const * font_tex = m_asset_manager.GetTexture(font_atlas.GetTextureId());
	if (font_tex)
	{
		font_tex_width = font_tex->GetWidth();
		font_tex_height = font_tex->GetHeight();
	}

	auto text_mesh = std::make_unique<TextMesh>(m_render_context, text,
		font_atlas, font_tex_width, font_tex_height, font_size, origin, viewport_width, viewport_height);
	text_mesh->SetUpdateMeshCallback([&asset_manager = m_asset_manager](AssetId id, dh::Mesh new_mesh)
		{
			if (!id.IsValid())
			{
				std::cout << "Scene::create_text_mesh: Invalid AssetId for updating mesh" << std::endl;
				return;
			}

			dh::Mesh * mesh = asset_manager.GetMesh(id);
			if (!mesh)
			{
				std::cout << "Scene::create_text_mesh: No mesh found in pool for AssetId: " << id.GetIndex() << std::endl;
				return;
			}

			*mesh = std::move(new_mesh);
		});

	std::expected<dh::Mesh, dh::GraphicsError> mesh = text_mesh->CreateMesh();
	if (!mesh.has_value())
	{
		std::cout << "Scene::create_text_mesh: Failed to create mesh. Error: "
			<< mesh.error().GetMessage() << std::endl;
		return text_mesh;
	}

	MeshId<TextMesh::VertexT> mesh_id = m_asset_manager.AddMesh<TextMesh::VertexT>(std::move(mesh.value()));
	text_mesh->SetMeshId(mesh_id);
	return text_mesh;
}

Scene::Scene(dh::RenderContext const & render_context, std::string const & title, float dpi_scale_factor)
	: m_render_context{ render_context }
	, m_title{ title }
	, m_asset_manager{ render_context }
	, m_renderer{ render_context, m_asset_manager }
	, m_camera3d{ render_context.ShouldFlipScreenY() }
	, m_camera2d{ render_context.ShouldFlipScreenY() }
{
	const glm::vec3 camera_pos{ 0.0f, -6.0f, 1.0f };
	const glm::vec3 camera_dir = glm::normalize(glm::vec3{ 0.0f, 5.0f, 0.0f } - camera_pos);
	m_camera3d.Init(camera_pos, camera_dir);

	const float label_font_size = 18.0f * dpi_scale_factor;
	const float title_font_size = 32.0f * dpi_scale_factor;

	// background
	AssetId bg_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetTexturesPath() / "forest_path.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	m_background.Init(m_asset_manager, bg_tex_id);
	const auto bg_pipeline_id = m_asset_manager.AddPipeline<BackgroundTexPipeline>(m_camera2d, m_asset_manager, bg_tex_id);
	m_renderer.CreateRenderObject("background", m_background.GetMeshId(), bg_pipeline_id);

	m_bounds.SetVertices({
		{-0.5f, -4.0f},
		{0.5f, -4.0f},
		{1.0f, 10.0f},
		{-1.0f, 10.0f},
	});

	// title and fps
	AssetId arial_tex_id = m_asset_manager.AddTexture(m_asset_manager.GetFontsPath() / "ArialAtlas.png",
		dh::PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_arial_font = std::make_unique<FontAtlas>(arial_tex_id, m_asset_manager.GetFontsPath() / "ArialAtlas.json");

	const auto text_pipeline_id = m_asset_manager.AddPipeline<TextPipeline>(m_camera2d, m_asset_manager, arial_tex_id);

	m_fps_mesh = create_text_mesh("FPS: ", *m_arial_font, label_font_size, glm::vec2{ -0.9, -0.9 } /*origin*/,
		0 /*viewport_width*/, 0 /*viewport_height*/);
	m_fps_label = TextPipeline::ObjectData{
		.screen_px_range = m_fps_mesh->GetScreenPxRange(),
		.bg_color = { 0.0f, 0.0f, 0.0f, 0.0f },
		.text_color = { 1.0f, 1.0f, 0.0f, 1.0 },
	};
	m_renderer.CreateRenderObject("fps label", m_fps_mesh->GetMeshId(), text_pipeline_id, m_fps_label);

	m_title_mesh = create_text_mesh(m_title, *m_arial_font, title_font_size, glm::vec2{ -0.9, 0.8 } /*origin*/,
		0 /*viewport_width*/, 0 /*viewport_height*/);
	m_title_label = TextPipeline::ObjectData{
		.screen_px_range = m_title_mesh->GetScreenPxRange(),
		.bg_color = { 0.0f, 0.0f, 0.0f, 0.0f },
		.text_color = { 1.0f, 1.0f, 1.0f, 1.0 },
	};
	m_renderer.CreateRenderObject("title", m_title_mesh->GetMeshId(), text_pipeline_id, m_title_label);

	// editor grid
	m_grid.Init(m_asset_manager);
	const auto line_pipeline_id = m_asset_manager.AddPipeline<LinePipeline>(m_camera3d);
	m_grid.SetRO(m_renderer.CreateRenderObject("grid", m_grid.GetMeshId(), line_pipeline_id));

	// dog
	m_dog.Init(m_asset_manager, camera_dir);
	// at some point, we should allow changing the pipeline's texture so we can reuse this for other sprites
	const auto dog_sprite_pipeline_id = m_asset_manager.AddPipeline<SpritePipeline>(m_camera3d, m_asset_manager, m_dog.GetTextureId());
	m_renderer.CreateRenderObject("dog", m_dog.GetMeshId(), dog_sprite_pipeline_id, m_dog.GetSpriteData());

	// baby
	m_baby.Init(m_asset_manager, camera_dir);
	const auto baby_sprite_pipeline_id = m_asset_manager.AddPipeline<SpritePipeline>(m_camera3d, m_asset_manager, m_baby.GetTextureId());
	m_renderer.CreateRenderObject("baby", m_baby.GetMeshId(), baby_sprite_pipeline_id, m_baby.GetSpriteData());
}

void Scene::OnViewportResized(int width, int height)
{
	m_camera3d.OnViewportResized(width, height);
	m_camera2d.OnViewportResized(width, height);

	// keep UI elements proportional to the height of the view
	m_background.OnViewportResized(width, height, m_asset_manager);
	if (m_fps_mesh)
		m_fps_mesh->OnViewportResized(width, height);
	if (m_title_mesh)
		m_title_mesh->OnViewportResized(width, height);
}

void Scene::OnDPIScalingFactorChanged(float dpi_scale_factor)
{
	float label_font_size = 18.0f * dpi_scale_factor;
	float title_font_size = 32.0f * dpi_scale_factor;

	if (m_fps_mesh)
		m_fps_mesh->SetFontSize(label_font_size);
	if (m_title_mesh)
		m_title_mesh->SetFontSize(title_font_size);
}

bool Scene::Update(float dt, Input const & input)
{
	if (input.KeyJustPressed(Input::Key::Esc))
		return false;

	m_camera3d.Update(dt, input);

	m_frame_timer += dt;
	m_frame_count++;
	if (m_frame_timer >= 1.0)
	{
		float fps = static_cast<float>(m_frame_count) / m_frame_timer;
		m_fps_mesh->SetText("FPS: " + std::to_string(static_cast<int>(fps)));
		m_frame_timer = 0.0;
		m_frame_count = 0;
	}

	m_grid.Update(input, m_renderer);
	m_dog.Update(dt, input, m_bounds);
	m_baby.Update(dt, &m_dog);

	return true;
}

void Scene::Render() const
{
	m_renderer.Render();
}
