// Scene.cpp

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

module Scene;

import LinePipeline;
import PlatformUtils;
import StbImage;
import TextMesh;

AssetId create_texture(
	AssetPool<Texture> & texture_pool,
	RenderContext const & render_context,
	std::filesystem::path const & filepath,
	PixelFormat format = PixelFormat::RGBA_SRGB,
	bool flip_vertically = false,
	bool use_mip_map = true)
{
	StbImage image(filepath, GetPixelSize(format) /*req_comp*/, flip_vertically);
	if (!image.IsValid())
	{
		std::cout << "Failed to load image: " << filepath << std::endl;
		return AssetId{};
	}

	Texture texture;
	std::expected<void, GraphicsError> result = texture.Create(
		render_context,
		ImageData{
			.data = image.GetData(),
			.format = format,
			.width = static_cast<std::uint32_t>(image.GetWidth()),
			.height = static_cast<std::uint32_t>(image.GetHeight())
		});
	if (!result.has_value() || !texture.IsValid())
	{
		std::cout << "Failed to create texture from image: " << filepath << std::endl;
		return AssetId{};
	}

	AssetId texture_id = texture_pool.Add(std::move(texture));
	if (!texture_id.IsValid())
		std::cout << "Failed to add texture to pool." << std::endl;

	return texture_id;
}

AssetId Scene::create_texture(
	std::filesystem::path const & filepath,
	PixelFormat format /*= PixelFormat::RGBA_SRGB*/,
	bool flip_vertically /*= false*/,
	bool use_mip_map /*= true*/)
{
	return ::create_texture(m_texture_pool, m_render_context, filepath, format, flip_vertically, use_mip_map);
}

MeshId<Texture2dVertex> Scene::create_bg_mesh()
{
	std::vector<Texture2dVertex> verts{
		{ { -1.0,  1.0 }, { 0.0, 1.0 } },
		{ {  1.0,  1.0 }, { 1.0, 1.0 } },
		{ { -1.0, -1.0 }, { 0.0, 0.0 } },
		{ {  1.0, -1.0 }, { 1.0, 0.0 } } };

	std::vector<Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	return create_mesh<Texture2dVertex>(verts, indices);
}

void Scene::resize_bg_mesh(MeshId<Texture2dVertex> mesh_id)
{
	Mesh * mesh = m_mesh_manager.Get(mesh_id);
	if (!mesh)
		return;

	Texture const * bg_tex = m_texture_pool.Get(m_bg_tex_id);
	if (!bg_tex)
		return;

	float world_scale = (static_cast<float>(m_view_height) / bg_tex->GetHeight());

	float y_size = 2.0f;
	float y_pos = -1.0f;
	float x_size = bg_tex->GetWidth() * (2.0f / m_view_width) * world_scale;
	float x_pos = -x_size / 2.0f;

	std::vector<Texture2dVertex> verts{
		{ { x_pos,          y_pos + y_size }, { 0.0, 1.0 } },
		{ { x_pos + x_size, y_pos + y_size }, { 1.0, 1.0 } },
		{ { x_pos,          y_pos          }, { 0.0, 0.0 } },
		{ { x_pos + x_size, y_pos          }, { 1.0, 0.0 } } };

	std::vector<Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	*mesh = Mesh{ m_render_context };
	std::expected<void, GraphicsError> result = mesh->Create(verts, indices);
	if (!result.has_value())
		std::cout << "Scene::resize_bg_mesh: Failed to create mesh. Error: " << result.error().GetMessage() << std::endl;
}

std::unique_ptr<TextMesh> Scene::create_text_mesh(
	std::string const & text,
	FontAtlas const & font_atlas,
	float font_size,
	glm::vec2 origin,
	int viewport_width,
	int viewport_height)
{
	std::uint32_t font_tex_width = 0, font_tex_height = 0;
	Texture const * font_tex = m_texture_pool.Get(font_atlas.GetTexture());
	if (font_tex)
	{
		font_tex_width = font_tex->GetWidth();
		font_tex_height = font_tex->GetHeight();
	}

	auto text_mesh = std::make_unique<TextMesh>(m_render_context, text,
		font_atlas, font_tex_width, font_tex_height, font_size, origin, viewport_width, viewport_height);
	text_mesh->SetUpdateMeshCallback([&mesh_manager = m_mesh_manager](AssetId id, Mesh new_mesh)
		{
			if (!id.IsValid())
			{
				std::cout << "Scene::create_text_mesh: Invalid AssetId for updating mesh" << std::endl;
				return;
			}

			Mesh * mesh = mesh_manager.Get(id);
			if (!mesh)
			{
				std::cout << "Scene::create_text_mesh: No mesh found in pool for AssetId: " << id.GetIndex() << std::endl;
				return;
			}

			*mesh = std::move(new_mesh);
		});

	std::expected<Mesh, GraphicsError> mesh = text_mesh->CreateMesh();
	if (!mesh.has_value())
	{
		std::cout << "Scene::create_text_mesh: Failed to create mesh. Error: "
			<< mesh.error().GetMessage() << std::endl;
		return text_mesh;
	}

	std::expected<MeshId<TextMesh::VertexT>, GraphicsError> mesh_id = m_mesh_manager.AddMesh<TextMesh::VertexT>(std::move(mesh.value()));
	if (!mesh_id.has_value() || !mesh_id.value().IsValid())
	{
		std::cout << "Failed to add text mesh to mesh manager: " << mesh_id.error().GetMessage() << std::endl;
		return text_mesh;
	}

	text_mesh->SetMeshId(mesh_id.value());
	return text_mesh;
}

MeshId<Texture2dVertex> Scene::create_sprite_mesh()
{
	std::vector<Texture2dVertex> verts{
		{ { -1.0,  1.0 }, { 0.0, 1.0 } },
		{ {  1.0,  1.0 }, { 1.0, 1.0 } },
		{ { -1.0, -1.0 }, { 0.0, 0.0 } },
		{ {  1.0, -1.0 }, { 1.0, 0.0 } } };

	std::vector<Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	return create_mesh<Texture2dVertex>(verts, indices);
}

void Scene::resize_sprite_mesh(MeshId<Texture2dVertex> mesh_id, SpriteSheet const & sprite_sheet)
{
	Mesh * mesh = m_mesh_manager.Get(mesh_id);
	if (!mesh)
		return;

	float aspect_ratio = static_cast<float>(sprite_sheet.GetFrameWidth()) / sprite_sheet.GetFrameHeight();
	float y_size = 1.0f;
	float y_pos = 0.0f;
	float x_size = 1.0f * aspect_ratio;
	float x_pos = -x_size / 2.0f;

	std::vector<Texture2dVertex> verts{
		{ { x_pos,          y_pos + y_size }, { 0.0, 0.0 } },
		{ { x_pos + x_size, y_pos + y_size }, { 1.0, 0.0 } },
		{ { x_pos,          y_pos          }, { 0.0, 1.0 } },
		{ { x_pos + x_size, y_pos          }, { 1.0, 1.0 } } };

	std::vector<Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	*mesh = Mesh{ m_render_context };
	std::expected<void, GraphicsError> result = mesh->Create(verts, indices);
	if (!result.has_value())
		std::cout << "Scene::resize_sprite_mesh: Failed to create mesh. Error: " << result.error().GetMessage() << std::endl;
}

MeshId<Vertex2d> Scene::create_grid_mesh()
{
	std::vector<Vertex2d> verts{
		{ { -1.0,  1.0 } },
		{ {  1.0,  1.0 } },
		{ { -1.0, -1.0 } },
		{ {  1.0, -1.0 } } };

	std::vector<Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	std::vector<LineInstance> line_instances(20);
	for (int y = 0; y < 10; y++)
	{
		line_instances[y].p0 = { -4.0f, static_cast<float>(y) - 4.0f, 0.0f };
		line_instances[y].p1 = { 5.0f, static_cast<float>(y) - 4.0f, 0.0f };
		line_instances[y].thickness = 4.0f;
		line_instances[y].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	}
	for (int x = 0; x < 10; x++)
	{
		line_instances[x + 10].p0 = { static_cast<float>(x) - 4.0f, -4.0f, 0.0f };
		line_instances[x + 10].p1 = { static_cast<float>(x) - 4.0f, 5.0f, 0.0f };
		line_instances[x + 10].thickness = 4.0f;
		line_instances[x + 10].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	return create_mesh<Vertex2d>(verts, indices, line_instances);
}

Scene::Scene(RenderContext const & render_context, std::string const & title, float dpi_scale_factor)
	: m_render_context{ render_context }
	, m_resources_path{ PlatformUtils::GetExecutableDir() / "resources" }
	, m_title{ title }
	, m_renderer{ render_context }
	, m_camera{ render_context.ShouldFlipScreenY() }
	, m_mesh_manager{ render_context }
{
	m_renderer.SetClearColor(glm::vec3{ 0.0f, 0.0f, 0.0f });

	const glm::vec3 camera_pos{ 0.0f, -6.0f, 1.0f };
	const glm::vec3 camera_dir = glm::normalize(glm::vec3{ 0.0f, 5.0f, 0.0f } - camera_pos);
	m_camera.Init(camera_pos, camera_dir);

	const float label_font_size = 18.0f * dpi_scale_factor;
	const float title_font_size = 32.0f * dpi_scale_factor;

	const std::filesystem::path textures_path = m_resources_path / "textures";
	const std::filesystem::path fonts_path = m_resources_path / "fonts";

	m_bg_tex_id = create_texture(textures_path / "forest_path.png");

	AssetId arial_tex_id = create_texture(fonts_path / "ArialAtlas.png", PixelFormat::RGB_UNORM, true /*flip_vertically*/, false /*use_mip_map*/);
	m_arial_font = std::make_unique<FontAtlas>(arial_tex_id, fonts_path / "ArialAtlas.json");

	Texture2dPipeline bg_pipeline = create_pipeline<Texture2dPipeline>(m_texture_pool, m_bg_tex_id);
	TextPipeline text_pipeline = create_pipeline<TextPipeline>(m_texture_pool, arial_tex_id);

	m_bg_mesh_id = create_bg_mesh();
	create_render_object("background", m_bg_mesh_id, bg_pipeline);

	m_fps_mesh = create_text_mesh("FPS: ", *m_arial_font, label_font_size, glm::vec2{ -0.9, -0.9 } /*origin*/,
		0 /*viewport_width*/, 0 /*viewport_height*/);
	m_fps_label = TextPipeline::ObjectData{
		.screen_px_range = m_fps_mesh->GetScreenPxRange(),
		.bg_color = { 0.0f, 0.0f, 0.0f, 0.0f },
		.text_color = { 1.0f, 1.0f, 0.0f, 1.0 },
	};
	create_render_object("fps label", m_fps_mesh->GetMeshId(), text_pipeline, m_fps_label);

	m_title_mesh = create_text_mesh(m_title, *m_arial_font, title_font_size, glm::vec2{ -0.9, 0.8 } /*origin*/,
		0 /*viewport_width*/, 0 /*viewport_height*/);
	m_title_label = TextPipeline::ObjectData{
		.screen_px_range = m_title_mesh->GetScreenPxRange(),
		.bg_color = { 0.0f, 0.0f, 0.0f, 0.0f },
		.text_color = { 1.0f, 1.0f, 1.0f, 1.0 },
	};
	create_render_object("title", m_title_mesh->GetMeshId(), text_pipeline, m_title_label);

	LinePipeline line_pipeline = create_pipeline<LinePipeline>(m_camera);
	const auto grid_mesh_id = create_grid_mesh();
	create_render_object("grid", grid_mesh_id, line_pipeline);

	const auto dog_tex_id = create_texture(textures_path / "dog_walk.png",
		 PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	SpritesheetPipeline dog_sprite_pipeline = create_pipeline<SpritesheetPipeline>(m_camera, m_texture_pool, dog_tex_id);
	const auto dog_mesh_id = create_sprite_mesh();
	const auto dog_render_object_id = create_render_object("dog", dog_mesh_id, dog_sprite_pipeline, m_dog.GetSpriteData());
	m_dog.Init(dog_tex_id, dog_mesh_id, dog_render_object_id, camera_dir);
}

void Scene::OnViewportResized(int width, int height)
{
	m_view_width = width;
	m_view_height = height;

	m_camera.OnViewportResized(width, height);

	resize_bg_mesh(m_bg_mesh_id);
	resize_sprite_mesh(m_dog.GetMeshId(), m_dog.GetSpriteSheet());

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
	if (input.KeyIsPressed(Input::Key::Esc))
		return false;

	m_timer += dt;

	m_frame_timer += dt;
	m_frame_count++;
	if (m_frame_timer >= 1.0)
	{
		float fps = static_cast<float>(m_frame_count) / m_frame_timer;
		m_fps_mesh->SetText("FPS: " + std::to_string(static_cast<int>(fps)));
		m_frame_timer = 0.0;
		m_frame_count = 0;
	}

	m_dog.Update(dt, input);

	m_camera.Update(dt, input);

	return true;
}

void Scene::Render() const
{
	m_renderer.BeginDraw();

	for (PipelineRenderObjects const & pipeline_r_objs : m_active_render_objects)
	{
		Pipeline const * pipeline = m_pipeline_pool.Get(pipeline_r_objs.pipeline_id);
		if (!pipeline)
		{
			std::cout << "Scene::Render: No pipeline found in pool for pipeline ID: " << pipeline_r_objs.pipeline_id.GetIndex() << std::endl;
			continue;
		}

		pipeline->Activate();
		pipeline->UpdatePerFrameConstants();

		for (AssetId obj_id : pipeline_r_objs.render_object_ids)
		{
			RenderObject const * obj = m_render_object_pool.Get(obj_id);
			if (!obj)
			{
				std::cout << "Scene::Render: No render object found in pool for AssetId: " << obj_id.GetIndex() << std::endl;
				continue;
			}

			Mesh const * mesh = m_mesh_manager.Get(obj->GetMeshId());
			if (!mesh)
			{
				std::cout << "Scene::Render: No mesh found in pool for AssetId: " << obj->GetMeshId().GetIndex() << std::endl;
				continue;
			}

			pipeline->UpdatePerObjectConstants(obj->GetObjectData());
			mesh->Render();
		}
	}

	m_renderer.EndDraw();
}
