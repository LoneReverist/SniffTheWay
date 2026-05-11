// TexturePipeline.ixx

module;

#include <expected>
#include <filesystem>

#include <glm/mat4x4.hpp>

export module TexturePipeline;

import Dreamhearth;

import AssetPool;
import Vertex;

using namespace Dreamhearth;

export class Texture2dPipeline
{
public:
	using VertexT = Texture2dVertex;

	static std::expected<Pipeline, GraphicsError> CreatePipeline(
		RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		AssetPool<Texture> const & texture_pool,
		AssetId texture_id);

	Texture2dPipeline() = default;
	explicit Texture2dPipeline(AssetId asset_id) : m_asset_id(asset_id) {}

	AssetId GetAssetId() const { return m_asset_id; }

private:
	AssetId m_asset_id;
};

std::expected<Pipeline, GraphicsError> Texture2dPipeline::CreatePipeline(
	RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	AssetPool<Texture> const & texture_pool,
	AssetId texture_id)
{
	struct ObjectDataVS
	{
		alignas(16) glm::mat4 model;
	};

	Texture const * texture = texture_pool.Get(texture_id);
	if (!texture)
		return std::unexpected{ GraphicsError{ "Texture2dPipeline::CreatePipeline: invalid texture" } };

	PipelineBuilder builder{ render_context };

	std::expected<void, GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "texture2d.vert",
		shaders_path / "texture2d.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetObjectDataTypes<ObjectDataVS, std::nullopt_t>();
	builder.SetTexture(*texture);
	builder.SetCullMode(CullMode::NONE);

	return builder.CreatePipeline();
}
