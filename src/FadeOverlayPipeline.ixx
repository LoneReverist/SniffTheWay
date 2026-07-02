// FadeOverlayPipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <optional>

#include <glm/vec4.hpp>
#include <glog/logging.h>

export module FadeOverlayPipeline;

import Dreamhearth;

import Camera;
import Vertex;

namespace dh = Dreamhearth;

export class FadeOverlayPipeline
{
public:
	using VertexT = ColorVertex2d;

	struct ObjectData
	{
		glm::vec4 color{ 0.0f, 0.0f, 0.0f, 1.0f };
	};

	static std::expected<dh::Pipeline, dh::GraphicsError> CreatePipeline(
		dh::RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera2d const & camera2d);

private:
	FadeOverlayPipeline() = delete;
};

std::expected<dh::Pipeline, dh::GraphicsError> FadeOverlayPipeline::CreatePipeline(
	dh::RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera2d const & camera2d)
{
	struct ObjectDataFS
	{
		alignas(16) glm::vec4 color{ 0.0f, 0.0f, 0.0f, 1.0f };
	};

	dh::PipelineBuilder builder{ render_context };

	std::expected<void, dh::GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "color2d.vert",
		shaders_path / "fade_overlay.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetVSUniformTypes<ProjUniform>();
	builder.SetObjectDataTypes<std::nullopt_t, ObjectDataFS>();
	builder.SetDepthTestOptions(dh::DepthTestOptions{
		.enable_depth_test = false,
		.enable_depth_write = false,
		.depth_compare_op = dh::DepthCompareOp::ALWAYS
		});
	builder.SetBlendOptions(dh::BlendOptions{
		.enable_blend = true,
		.src_factor = dh::BlendFactor::SRC_ALPHA,
		.dst_factor = dh::BlendFactor::ONE_MINUS_SRC_ALPHA
		});
	builder.SetCullMode(dh::CullMode::BACK);

	builder.SetPerFrameConstantsCallback(
		[&camera2d](dh::Pipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera2d.GetProjUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[](dh::Pipeline const & pipeline, void const * object_data)
		{
			if (!object_data)
			{
				LOG_FIRST_N(ERROR, 10) << "FadeOverlayPipeline: ObjectData is null";
				return;
			}

			auto const * data = static_cast<ObjectData const *>(object_data);
			pipeline.SetObjectData(std::nullopt, ObjectDataFS{ .color = data->color });
		});

	return builder.CreatePipeline();
}
