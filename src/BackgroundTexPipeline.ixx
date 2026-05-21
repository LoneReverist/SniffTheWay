// BackgroundTexPipeline.ixx

module;

#include <expected>
#include <filesystem>

#include <glm/mat4x4.hpp>

export module BackgroundTexPipeline;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import AssetPool;
import Camera;
import Vertex;

export class BackgroundTexPipeline
{
public:
	using VertexT = TextureVertex2d;

	static std::expected<dh::Pipeline, dh::GraphicsError> CreatePipeline(
		dh::RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera2d const & camera2d,
		AssetManager const & asset_manager,
		AssetId texture_id);

private:
	BackgroundTexPipeline() = delete;
};

std::expected<dh::Pipeline, dh::GraphicsError> BackgroundTexPipeline::CreatePipeline(
	dh::RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera2d const & camera2d,
	AssetManager const & asset_manager,
	AssetId texture_id)
{
	struct ObjectDataVS
	{
		alignas(16) glm::mat4 model{ 1.0f };
	};

	dh::Texture const * texture = asset_manager.GetTexture(texture_id);
	if (!texture)
		return std::unexpected{ dh::GraphicsError{ "BackgroundTexPipeline::CreatePipeline: invalid texture" } };

	dh::PipelineBuilder builder{ render_context };

	std::expected<void, dh::GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "texture2d.vert",
		shaders_path / "texture2d.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetVSUniformTypes<ProjUniform>();
	builder.SetObjectDataTypes<ObjectDataVS, std::nullopt_t>();
	builder.SetTexture(*texture);
	builder.SetCullMode(dh::CullMode::NONE);
	builder.SetDepthTestOptions(dh::DepthTestOptions{
		.enable_depth_test = false,
		.enable_depth_write = false,
		.depth_compare_op = dh::DepthCompareOp::ALWAYS
		});

	builder.SetPerFrameConstantsCallback(
		[&camera2d](dh::Pipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera2d.GetProjUniform());
		});

	return builder.CreatePipeline();
}
