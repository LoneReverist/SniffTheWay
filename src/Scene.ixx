// Scene.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

export module Scene;

import Dreamhearth;

import AssetPool;
import Camera;
import Dog;
import FontAtlas;
import Input;
import MeshManager;
import RenderObject;
import SpriteSheet;
import TextMesh;
import TextPipeline;
import Vertex;

using namespace Dreamhearth;

template <typename MeshT, typename PipelineT>
concept MeshIsCompatibleWithPipeline = std::same_as<typename MeshT::VertexT, typename PipelineT::VertexT>;

template <typename PipelineT>
concept PipelineHasObjectData = requires { typename PipelineT::ObjectData; };

template <typename ObjectDataT, typename PipelineT>
concept ObjectDataIsCompatibleWithPipeline = std::same_as<ObjectDataT, typename PipelineT::ObjectData>
|| (!PipelineHasObjectData<PipelineT> && std::same_as<ObjectDataT, std::nullopt_t>);

// Keeps track of which render objects are using the associated pipeline,
// this allows the render objects to be grouped by pipeline for more efficient rendering
struct PipelineRenderObjects
{
	AssetId pipeline_id;
	std::vector<AssetId> render_object_ids;
};

export class Scene
{
public:
	explicit Scene(RenderContext const & render_context, std::string const & title, float dpi_scale_factor);

	void OnViewportResized(int width, int height);
	void OnDPIScalingFactorChanged(float dpi_scale_factor);

	bool Update(float dt, Input const & input);
	void Render() const;

private:
	template <IsVertex VertexT, typename... Args>
	MeshId<VertexT> create_mesh(Args &&... args);

	template <typename PipelineT, typename... Args>
	PipelineT create_pipeline(Args &&... args);

	template <IsVertex VertexT, typename PipelineT, typename ObjectDataT = std::nullopt_t>
		requires MeshIsCompatibleWithPipeline<MeshId<VertexT>, PipelineT> && ObjectDataIsCompatibleWithPipeline<ObjectDataT, PipelineT>
	AssetId create_render_object(
		std::string const & name,
		MeshId<VertexT> const & mesh_id,
		PipelineT const & pipeline,
		ObjectDataT const & object_data = std::nullopt);

	AssetId create_texture(
		std::filesystem::path const & filepath,
		PixelFormat format = PixelFormat::RGBA_SRGB,
		bool flip_vertically = false,
		bool use_mip_map = true);

	MeshId<TextureVertex2d> create_bg_mesh();
	void resize_bg_mesh(MeshId<TextureVertex2d> mesh_id);

	MeshId<TextureVertex2d> create_sprite_mesh();
	void resize_sprite_mesh(MeshId<TextureVertex2d> mesh_id, SpriteSheet const & sprite_sheet);

	MeshId<Vertex2d> create_grid_mesh();

	std::unique_ptr<TextMesh> create_text_mesh(
		std::string const & text,
		FontAtlas const & font_atlas,
		float font_size,
		glm::vec2 origin,
		int viewport_width,
		int viewport_height);

private:
	RenderContext const & m_render_context;
	std::filesystem::path const m_resources_path;
	std::string const m_title;

	Renderer m_renderer;
	Camera3d m_camera3d;
	Camera2d m_camera2d;
	int m_view_width = 0;
	int m_view_height = 0;

	MeshManager m_mesh_manager;
	AssetPool<Pipeline> m_pipeline_pool;
	AssetPool<RenderObject> m_render_object_pool;
	AssetPool<Texture> m_texture_pool;

	std::vector<PipelineRenderObjects> m_active_render_objects;

	AssetId m_bg_tex_id;
	MeshId<TextureVertex2d> m_bg_mesh_id;

	std::unique_ptr<FontAtlas> m_arial_font;

	std::unique_ptr<TextMesh> m_fps_mesh;
	std::unique_ptr<TextMesh> m_title_mesh;

	TextPipeline::ObjectData m_fps_label;
	TextPipeline::ObjectData m_title_label;

	Dog m_dog;

	float m_frame_timer = 0.0f;
	int m_frame_count = 0;
};

template<IsVertex VertexT, typename... Args>
MeshId<VertexT> Scene::create_mesh(Args &&... args)
{
	std::expected<MeshId<VertexT>, GraphicsError> mesh_id
		= m_mesh_manager.CreateMesh<VertexT>(std::forward<Args>(args)...);
	if (!mesh_id.has_value())
	{
		std::cout << "Failed to create mesh: " << mesh_id.error().GetMessage() << std::endl;
		return MeshId<VertexT>{};
	}

	return mesh_id.value();
}

template <typename PipelineT, typename... Args>
PipelineT Scene::create_pipeline(Args &&... args)
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";

	std::expected<Pipeline, GraphicsError> pipeline
		= PipelineT::CreatePipeline(m_render_context, shaders_path, std::forward<Args>(args)...);
	if (!pipeline.has_value())
	{
		std::cout << "Failed to create " << typeid(PipelineT).name()
			<< " Error: " << pipeline.error().GetMessage() << std::endl;
		return PipelineT{};
	}

	AssetId pipeline_id = m_pipeline_pool.Add(std::move(pipeline.value()));
	if (!pipeline_id.IsValid())
		std::cout << "Failed to add pipeline to pool." << std::endl;

	return PipelineT{ pipeline_id };
}

template <IsVertex VertexT, typename PipelineT, typename ObjectDataT /*= std::nullopt_t*/>
	requires MeshIsCompatibleWithPipeline<MeshId<VertexT>, PipelineT> && ObjectDataIsCompatibleWithPipeline<ObjectDataT, PipelineT>
AssetId Scene::create_render_object(
	std::string const & name,
	MeshId<VertexT> const & mesh_id,
	PipelineT const & pipeline,
	ObjectDataT const & object_data /*= std::nullopt*/)
{
	if (!mesh_id.IsValid())
	{
		std::cout << "Scene::create_render_object: invalid mesh id for object: " + name;
		return AssetId{};
	}
	if (!pipeline.GetAssetId().IsValid())
	{
		std::cout << "Scene::create_render_object: invalid pipeline id for object: " + name;
		return AssetId{};
	}

	RenderObject obj{ name, mesh_id, pipeline.GetAssetId() };
	if constexpr (!std::same_as<ObjectDataT, std::nullopt_t>)
		obj.SetObjectData(&object_data);

	AssetId obj_id = m_render_object_pool.Add(std::move(obj));
	if (!obj_id.IsValid())
	{
		std::cout << "Failed to add render object to pool for object: " + name;
		return obj_id;
	}

	auto iter = std::ranges::find(m_active_render_objects, pipeline.GetAssetId(), &PipelineRenderObjects::pipeline_id);
	if (iter != m_active_render_objects.end())
		iter->render_object_ids.push_back(obj_id);
	else
		m_active_render_objects.push_back(PipelineRenderObjects{ pipeline.GetAssetId(), { obj_id } });

	return obj_id;
}
