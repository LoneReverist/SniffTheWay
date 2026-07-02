// TextureMaskPipeline.ixx

module;

#include <expected>
#include <filesystem>

#include <glog/logging.h>

export module TextureMaskPipeline;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Camera;
import Vertex;

namespace dh = Dreamhearth;

export class TextureMaskPipeline
{
public:
	using VertexT = TextureVertex2d;

	struct ObjectData
	{
		AssetId tex_id;
	};

	static std::expected<dh::Pipeline, dh::GraphicsError> CreatePipeline(
		dh::RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera2d const & camera2d,
		AssetManager const & asset_manager);

private:
	TextureMaskPipeline() = delete;
};

std::expected<dh::Pipeline, dh::GraphicsError> TextureMaskPipeline::CreatePipeline(
	dh::RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera2d const & camera2d,
	AssetManager const & asset_manager)
{
	dh::PipelineBuilder builder{ render_context };

	std::expected<void, dh::GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "texture2d.vert",
		shaders_path / "texture_mask.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetVSUniformTypes<ProjUniform>();
	builder.SetHasTexture(true);
	builder.SetCullMode(dh::CullMode::NONE);
	builder.SetDepthTestOptions(dh::DepthTestOptions{
		.uses_depth_attachment = false,
		.enable_depth_test = false,
		.enable_depth_write = false,
		.depth_compare_op = dh::DepthCompareOp::ALWAYS
		});
	builder.SetBlendOptions(dh::BlendOptions{
		.enable_blend = true,
		.src_factor = dh::BlendFactor::SRC_ALPHA,
		.dst_factor = dh::BlendFactor::ONE_MINUS_SRC_ALPHA,
		});

	builder.SetPerFrameConstantsCallback(
		[&camera2d](dh::Pipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera2d.GetProjUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[&asset_manager](dh::Pipeline const & pipeline, void const * object_data)
		{
			if (!object_data)
			{
				LOG_FIRST_N(ERROR, 10) << "TextureMaskPipeline: ObjectData is null";
				return;
			}

			auto const * data = static_cast<ObjectData const *>(object_data);

			dh::Texture const * texture = asset_manager.GetTexture(data->tex_id);
			if (texture)
			{
				pipeline.BindTexture(0, *texture);
			}
			else
			{
				LOG_FIRST_N(ERROR, 10) << "TextureMaskPipeline: No texture found in pool for AssetId: " << data->tex_id.GetIndex();
			}
		});

	return builder.CreatePipeline();
}
