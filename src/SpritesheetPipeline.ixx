// SpritesheetPipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>

#include <glm/vec4.hpp>

export module SpritesheetPipeline;

import Dreamhearth;

import AssetPool;
import Vertex;

using namespace Dreamhearth;

export class SpritesheetPipeline
{
public:
	using VertexT = Texture2dVertex;

	struct ObjectData
	{
		glm::vec4 frame_uvs; // x = min_u, y = max_u, z = min_v, w = max_v
	};

	static std::expected<Pipeline, GraphicsError> CreatePipeline(
		RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		AssetPool<Texture> const & texture_pool,
		AssetId texture_id);

	SpritesheetPipeline() = default;
	explicit SpritesheetPipeline(AssetId asset_id) : m_asset_id(asset_id) {}

	AssetId GetAssetId() const { return m_asset_id; }

private:
	AssetId m_asset_id;
};

std::expected<Pipeline, GraphicsError> SpritesheetPipeline::CreatePipeline(
	RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	AssetPool<Texture> const & texture_pool,
	AssetId texture_id)
{
	struct ObjectDataFS
	{
		alignas(16) glm::vec4 frame_uvs;
	};

	Texture const * texture = texture_pool.Get(texture_id);
	if (!texture)
		return std::unexpected{ GraphicsError{ "SpritesheetPipeline::CreatePipeline: invalid texture" } };

	PipelineBuilder builder{ render_context };

	std::expected<void, GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "sprite.vert",
		shaders_path / "sprite.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetObjectDataTypes<std::nullopt_t, ObjectDataFS>();
	builder.SetTexture(*texture);
	builder.SetDepthTestOptions(DepthTestOptions{
		.enable_depth_test = false,
		.enable_depth_write = false,
		.depth_compare_op = DepthCompareOp::ALWAYS
		});
	builder.SetBlendOptions(BlendOptions{
		.enable_blend = true,
		.src_factor = BlendFactor::SRC_ALPHA,
		.dst_factor = BlendFactor::ONE_MINUS_SRC_ALPHA
		});
	builder.SetCullMode(CullMode::NONE);

	builder.SetPerObjectConstantsCallback(
		[](Pipeline const & pipeline, void const * object_data)
		{
			if (!object_data)
			{
				std::cout << "SpriteSheet ObjectData is null for SpritesheetPipeline" << std::endl;
				return;
			}

			auto const * data = static_cast<ObjectData const *>(object_data);

			pipeline.SetObjectData(
				std::nullopt,
				ObjectDataFS{
					.frame_uvs = data->frame_uvs
				});
		});

	return builder.CreatePipeline();
}
