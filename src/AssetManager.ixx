// AssetManager.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <vector>

export module AssetManager;

import Dreamhearth;
using namespace Dreamhearth;

import AssetPool;
import PlatformUtils;
import StbImage;
import Vertex;

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
	AssetManager(RenderContext const & render_context)
		: m_render_context{ render_context }
		, m_resources_path{ PlatformUtils::GetExecutableDir() / "resources" }
	{}

	template<IsVertex VertexT>
	MeshId<VertexT> AddMesh(Mesh mesh);

	template<IsVertex VertexT>
	MeshId<VertexT> AddMesh(
		std::vector<VertexT> const & vertices,
		std::vector<Mesh::IndexT> const & indices);

	template<IsVertex VertexT, typename InstanceDataT>
	MeshId<VertexT> AddMesh(
		std::vector<VertexT> const & vertices,
		std::vector<Mesh::IndexT> const & indices,
		std::vector<InstanceDataT> const & instance_data);

	template <typename PipelineT, typename... Args>
	PipelineId<PipelineT> AddPipeline(Args &&... args);

	AssetId AddTexture(
		std::filesystem::path const & filepath,
		PixelFormat format = PixelFormat::RGBA_SRGB,
		bool flip_vertically = false,
		bool use_mip_map = true);

	void RemoveMesh(AssetId id) { m_mesh_pool.Remove(id); }
	void RemovePipeline(AssetId id) { m_pipeline_pool.Remove(id); }
	void RemoveTexture(AssetId id) { m_texture_pool.Remove(id); }

	Mesh * GetMesh(AssetId id) { return m_mesh_pool.Get(id); }
	Mesh const * GetMesh(AssetId id) const { return m_mesh_pool.Get(id); }
	Pipeline const * GetPipeline(AssetId id) const { return m_pipeline_pool.Get(id); }
	Texture const * GetTexture(AssetId id) const { return m_texture_pool.Get(id); }

	std::filesystem::path const & GetResourcesPath() const { return m_resources_path; }
	std::filesystem::path GetShadersPath() const { return m_resources_path / "shaders"; }
	std::filesystem::path GetTexturesPath() const { return m_resources_path / "textures"; }
	std::filesystem::path GetFontsPath() const { return m_resources_path / "fonts"; }

private:
	RenderContext const & m_render_context;
	std::filesystem::path m_resources_path;

	AssetPool<Mesh> m_mesh_pool;
	AssetPool<Pipeline> m_pipeline_pool;
	AssetPool<Texture> m_texture_pool;
};

template<IsVertex VertexT>
MeshId<VertexT> AssetManager::AddMesh(Mesh mesh)
{
	MeshId<VertexT> mesh_id{ m_mesh_pool.Add(std::move(mesh)) };
	if (!mesh_id.IsValid())
		std::cout << "AssetManager::AddMesh: Failed to add mesh to pool." << std::endl;

	return mesh_id;
}

template<IsVertex VertexT>
MeshId<VertexT> AssetManager::AddMesh(
	std::vector<VertexT> const & vertices,
	std::vector<Mesh::IndexT> const & indices)
{
	Mesh mesh{ m_render_context };
	std::expected<void, GraphicsError> result = mesh.Create(vertices, indices);
	if (!result.has_value())
	{
		std::cout << "AssetManager::AddMesh: Failed to create mesh: " << result.error().GetMessage() << std::endl;
		return MeshId<VertexT>{};
	}

	return AddMesh<VertexT>(std::move(mesh));
}

template<IsVertex VertexT, typename InstanceDataT>
MeshId<VertexT> AssetManager::AddMesh(
	std::vector<VertexT> const & vertices,
	std::vector<Mesh::IndexT> const & indices,
	std::vector<InstanceDataT> const & instance_data)
{
	Mesh mesh{ m_render_context };
	std::expected<void, GraphicsError> result = mesh.Create(vertices, indices, instance_data);
	if (!result.has_value())
	{
		std::cout << "AssetManager::AddMesh: Failed to create mesh: " << result.error().GetMessage() << std::endl;
		return MeshId<VertexT>{};
	}

	return AddMesh<VertexT>(std::move(mesh));
}

template <typename PipelineT, typename... Args>
PipelineId<PipelineT> AssetManager::AddPipeline(Args &&... args)
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";

	std::expected<Pipeline, GraphicsError> pipeline
		= PipelineT::CreatePipeline(m_render_context, shaders_path, std::forward<Args>(args)...);
	if (!pipeline.has_value())
	{
		std::cout << "AssetManager::AddPipeline: Failed to create pipeline: " << pipeline.error().GetMessage() << std::endl;
		return PipelineId<PipelineT>{};
	}

	PipelineId<PipelineT> pipeline_id{ m_pipeline_pool.Add(std::move(pipeline.value())) };
	if (!pipeline_id.IsValid())
		std::cout << "AssetManager::AddPipeline: Failed to add pipeline to pool." << std::endl;

	return pipeline_id;
}

AssetId AssetManager::AddTexture(
	std::filesystem::path const & filepath,
	PixelFormat format /*= PixelFormat::RGBA_SRGB*/,
	bool flip_vertically /*= false*/,
	bool use_mip_map /*= true*/)
{
	StbImage image(filepath, GetPixelSize(format) /*req_comp*/, flip_vertically);
	if (!image.IsValid())
	{
		std::cout << "Failed to load image: " << filepath << std::endl;
		return AssetId{};
	}

	Texture texture;
	std::expected<void, GraphicsError> result = texture.Create(
		m_render_context,
		ImageData{
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
