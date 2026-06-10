// ShadowBackdropPipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

export module ShadowBackdropPipeline;

import Dreamhearth;

import Camera;
import Vertex;

namespace dh = Dreamhearth;

export class ShadowBackdropPipeline
{
public:
	using VertexT = TextureVertex2d;

	struct ObjectData
	{
		glm::vec2 inner_min_uv{ 0.0f };
		glm::vec2 inner_max_uv{ 1.0f };
		glm::vec4 color{ 0.0f, 0.0f, 0.0f, 0.8f };
	};

	static std::expected<dh::Pipeline, dh::GraphicsError> CreatePipeline(
		dh::RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera2d const & camera2d);

private:
	ShadowBackdropPipeline() = delete;
};

std::expected<dh::Pipeline, dh::GraphicsError> ShadowBackdropPipeline::CreatePipeline(
	dh::RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera2d const & camera2d)
{
	struct ObjectDataFS
	{
		alignas(8) glm::vec2 inner_min_uv{ 0.0f };
		alignas(8) glm::vec2 inner_max_uv{ 1.0f };
		alignas(16) glm::vec4 color{ 0.0f, 0.0f, 0.0f, 0.8f };
	};

	dh::PipelineBuilder builder{ render_context };

	std::expected<void, dh::GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "texture2d.vert",
		shaders_path / "shadow_backdrop.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetVSUniformTypes<ProjUniform>();
	builder.SetObjectDataTypes<std::nullopt_t, ObjectDataFS>();
	builder.SetCullMode(dh::CullMode::NONE);
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
				std::cout << "ShadowBackdropPipeline: ObjectData is null" << std::endl;
				return;
			}

			auto const * data = static_cast<ObjectData const *>(object_data);
			pipeline.SetObjectData(
				std::nullopt,
				ObjectDataFS{
					.inner_min_uv = data->inner_min_uv,
					.inner_max_uv = data->inner_max_uv,
					.color = data->color,
				});
		});

	return builder.CreatePipeline();
}
