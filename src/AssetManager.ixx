// AssetManager.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <array>
#include <string>
#include <vector>

export module AssetManager;

import Dreamhearth;

import AssetPool;
import PlatformUtils;
import StbImage;
import Vertex;

namespace dh = Dreamhearth;

export template<IsVertex T>
class MeshId : public AssetId
{
public:
	using VertexT = T;
};

export template<typename T>
class PipelineId : public AssetId
{
public:
	using PipelineT = T;
};

export template <typename MeshIdT, typename PipelineIdT>
concept MeshIsCompatibleWithPipeline = std::same_as<typename MeshIdT::VertexT, typename PipelineIdT::PipelineT::VertexT>;

export template <typename PipelineT>
concept PipelineHasObjectData = requires { typename PipelineT::ObjectData; };

export template <typename ObjectDataT, typename PipelineIdT>
concept ObjectDataIsCompatibleWithPipeline = std::same_as<ObjectDataT, typename PipelineIdT::PipelineT::ObjectData>
	|| (!PipelineHasObjectData<typename PipelineIdT::PipelineT> && std::same_as<ObjectDataT, std::nullopt_t>);

export class AssetManager
{
public:
	AssetManager(dh::RenderContext const & render_context)
		: m_render_context{ render_context }
		, m_resources_path{ PlatformUtils::GetExecutableDir() / "resources" }
	{}

	template<IsVertex VertexT>
	MeshId<VertexT> AddMesh(dh::Mesh mesh);

	template<IsVertex VertexT>
	MeshId<VertexT> AddMesh(
		std::vector<VertexT> const & vertices,
		std::vector<dh::Mesh::IndexT> const & indices);

	template<IsVertex VertexT, typename InstanceDataT>
	MeshId<VertexT> AddMesh(
		std::vector<VertexT> const & vertices,
		std::vector<dh::Mesh::IndexT> const & indices,
		std::vector<InstanceDataT> const & instance_data);

	template<IsVertex VertexT>
	void UpdateMesh(MeshId<VertexT> mesh_id, dh::Mesh new_mesh);

	template<IsVertex VertexT>
	void UpdateMesh(
		MeshId<VertexT> mesh_id,
		std::vector<VertexT> const & vertices,
		std::vector<dh::Mesh::IndexT> const & indices);

	template <typename PipelineT, typename... Args>
	PipelineId<PipelineT> AddPipeline(Args &&... args);

	AssetId AddTexture(
		std::filesystem::path const & filepath,
		dh::PixelFormat format = dh::PixelFormat::RGBA_SRGB,
		bool flip_vertically = false,
		bool use_mip_map = true);
	AssetId AddRenderTexture(std::uint32_t width, std::uint32_t height);

	void RemoveMesh(AssetId id);
	void RemovePipeline(AssetId id);
	void RemoveTexture(AssetId id);
	void DestroyPendingAssets() const;

	dh::Mesh * GetMesh(AssetId id) { return m_mesh_pool.Get(id); }
	dh::Mesh const * GetMesh(AssetId id) const { return m_mesh_pool.Get(id); }
	dh::Pipeline const * GetPipeline(AssetId id) const { return m_pipeline_pool.Get(id); }
	dh::Texture * GetTexture(AssetId id) { return m_texture_pool.Get(id); }
	dh::Texture const * GetTexture(AssetId id) const { return m_texture_pool.Get(id); }

	std::filesystem::path const & GetResourcesPath() const { return m_resources_path; }
	std::filesystem::path GetShadersPath() const { return m_resources_path / "shaders"; }
	std::filesystem::path GetTexturesPath() const { return m_resources_path / "textures"; }
	std::filesystem::path GetFontsPath() const { return m_resources_path / "fonts"; }

	dh::RenderContext const & GetRenderContext() const { return m_render_context; }

private:
	template <typename AssetT>
	using DeferredFrameAssets = std::array<std::vector<AssetT>, dh::RenderContext::MaxFramesInFlight>;

	void defer_destroy(dh::Mesh mesh) const;
	void defer_destroy(dh::Pipeline pipeline) const;
	void defer_destroy(dh::Texture texture) const;

private:
	dh::RenderContext const & m_render_context;
	std::filesystem::path m_resources_path;

	AssetPool<dh::Mesh> m_mesh_pool;
	AssetPool<dh::Pipeline> m_pipeline_pool;
	AssetPool<dh::Texture> m_texture_pool;

	mutable DeferredFrameAssets<dh::Mesh> m_meshes_to_destroy;
	mutable DeferredFrameAssets<dh::Pipeline> m_pipelines_to_destroy;
	mutable DeferredFrameAssets<dh::Texture> m_textures_to_destroy;
};

template<IsVertex VertexT>
MeshId<VertexT> AssetManager::AddMesh(dh::Mesh mesh)
{
	MeshId<VertexT> mesh_id{ m_mesh_pool.Add(std::move(mesh)) };
	if (!mesh_id.IsValid())
		std::cout << "AssetManager::AddMesh: Failed to add mesh to pool." << std::endl;

	return mesh_id;
}

template<IsVertex VertexT>
MeshId<VertexT> AssetManager::AddMesh(
	std::vector<VertexT> const & vertices,
	std::vector<dh::Mesh::IndexT> const & indices)
{
	dh::Mesh mesh{ m_render_context };
	std::expected<void, dh::GraphicsError> result = mesh.Create(vertices, indices);
	if (!result.has_value())
	{
		std::cout << "AssetManager::AddMesh: Failed to create mesh: " << result.error().GetMessage().c_str() << std::endl;
		return MeshId<VertexT>{};
	}

	return AddMesh<VertexT>(std::move(mesh));
}

template<IsVertex VertexT, typename InstanceDataT>
MeshId<VertexT> AssetManager::AddMesh(
	std::vector<VertexT> const & vertices,
	std::vector<dh::Mesh::IndexT> const & indices,
	std::vector<InstanceDataT> const & instance_data)
{
	dh::Mesh mesh{ m_render_context };
	std::expected<void, dh::GraphicsError> result = mesh.Create(vertices, indices, instance_data);
	if (!result.has_value())
	{
		std::cout << "AssetManager::AddMesh: Failed to create mesh: " << result.error().GetMessage().c_str() << std::endl;
		return MeshId<VertexT>{};
	}

	return AddMesh<VertexT>(std::move(mesh));
}

template<IsVertex VertexT>
void AssetManager::UpdateMesh(MeshId<VertexT> mesh_id, dh::Mesh new_mesh)
{
	dh::Mesh * mesh = GetMesh(mesh_id);
	if (!mesh)
		return;

	defer_destroy(std::move(*mesh));
	*mesh = std::move(new_mesh);
}

template<IsVertex VertexT>
void AssetManager::UpdateMesh(
	MeshId<VertexT> mesh_id,
	std::vector<VertexT> const & vertices,
	std::vector<dh::Mesh::IndexT> const & indices)
{
	dh::Mesh * mesh = GetMesh(mesh_id);
	if (!mesh)
		return;

	dh::Mesh new_mesh{ m_render_context };
	std::expected<void, dh::GraphicsError> result = new_mesh.Create(vertices, indices);
	if (!result.has_value())
	{
		std::cout << "AssetManager::UpdateMesh: Failed to create mesh. Error: " << result.error().GetMessage() << std::endl;
		return;
	}

	defer_destroy(std::move(*mesh));
	*mesh = std::move(new_mesh);
}

template <typename PipelineT, typename... Args>
PipelineId<PipelineT> AssetManager::AddPipeline(Args &&... args)
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";

	std::expected<dh::Pipeline, dh::GraphicsError> pipeline
		= PipelineT::CreatePipeline(m_render_context, shaders_path, std::forward<Args>(args)...);
	if (!pipeline.has_value())
	{
		std::cout << "AssetManager::AddPipeline: Failed to create pipeline: " << pipeline.error().GetMessage().c_str() << std::endl;
		return PipelineId<PipelineT>{};
	}

	PipelineId<PipelineT> pipeline_id{ m_pipeline_pool.Add(std::move(pipeline.value())) };
	if (!pipeline_id.IsValid())
		std::cout << "AssetManager::AddPipeline: Failed to add pipeline to pool." << std::endl;

	return pipeline_id;
}

void AssetManager::RemoveMesh(AssetId id)
{
	dh::Mesh * mesh = GetMesh(id);
	if (mesh)
		defer_destroy(std::move(*mesh));

	m_mesh_pool.Remove(id);
}

void AssetManager::RemovePipeline(AssetId id)
{
	dh::Pipeline * pipeline = m_pipeline_pool.Get(id);
	if (pipeline)
		defer_destroy(std::move(*pipeline));

	m_pipeline_pool.Remove(id);
}

void AssetManager::RemoveTexture(AssetId id)
{
	dh::Texture * texture = GetTexture(id);
	if (texture)
		defer_destroy(std::move(*texture));

	m_texture_pool.Remove(id);
}

void AssetManager::DestroyPendingAssets() const
{
	const std::uint32_t frame_index = m_render_context.GetCurFrameIndex();
	if (frame_index >= dh::RenderContext::MaxFramesInFlight)
		return;

	m_meshes_to_destroy[frame_index].clear();
	m_pipelines_to_destroy[frame_index].clear();
	m_textures_to_destroy[frame_index].clear();
}

void AssetManager::defer_destroy(dh::Mesh mesh) const
{
	m_meshes_to_destroy[m_render_context.GetCurFrameIndex()].push_back(std::move(mesh));
}

void AssetManager::defer_destroy(dh::Pipeline pipeline) const
{
	m_pipelines_to_destroy[m_render_context.GetCurFrameIndex()].push_back(std::move(pipeline));
}

void AssetManager::defer_destroy(dh::Texture texture) const
{
	m_textures_to_destroy[m_render_context.GetCurFrameIndex()].push_back(std::move(texture));
}

AssetId AssetManager::AddTexture(
	std::filesystem::path const & filepath,
	dh::PixelFormat format /*= dh::PixelFormat::RGBA_SRGB*/,
	bool flip_vertically /*= false*/,
	bool use_mip_map /*= true*/)
{
	StbImage image(filepath, GetPixelSize(format) /*req_comp*/, flip_vertically);
	if (!image.IsValid())
	{
		std::cout << "Failed to load image: " << filepath << std::endl;
		return AssetId{};
	}

	dh::Texture texture;
	std::expected<void, dh::GraphicsError> result = texture.Create(
		m_render_context,
		dh::ImageData{
			.data = image.GetData(),
			.format = format,
			.width = static_cast<std::uint32_t>(image.GetWidth()),
			.height = static_cast<std::uint32_t>(image.GetHeight())
		});
	if (!result.has_value() || !texture.IsValid())
	{
		std::cout << "AssetManager::AddTexture: Failed to create texture from image: " << filepath << std::endl;
		return AssetId{};
	}

	AssetId texture_id = m_texture_pool.Add(std::move(texture));
	if (!texture_id.IsValid())
		std::cout << "AssetManager::AddTexture: Failed to add texture to pool." << std::endl;

	return texture_id;
}

AssetId AssetManager::AddRenderTexture(std::uint32_t width, std::uint32_t height)
{
	dh::Texture texture;
	std::expected<void, dh::GraphicsError> result = texture.CreateRenderTarget(m_render_context, width, height);
	if (!result.has_value() || !texture.IsValid())
	{
		std::cout << "AssetManager::AddRenderTexture: Failed to create render texture." << std::endl;
		return AssetId{};
	}

	AssetId texture_id = m_texture_pool.Add(std::move(texture));
	if (!texture_id.IsValid())
		std::cout << "AssetManager::AddRenderTexture: Failed to add texture to pool." << std::endl;

	return texture_id;
}
