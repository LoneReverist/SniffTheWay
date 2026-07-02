// ScentTrailPipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glog/logging.h>

export module ScentTrailPipeline;

import Dreamhearth;

import Camera;
import Vertex;

namespace dh = Dreamhearth;

export class ScentTrailPipeline
{
public:
	using VertexT = ScentTrailVertex;

	struct ObjectData
	{
		glm::vec4 color{ 1.0f, 0.83f, 0.38f, 1.0f };
		glm::vec2 dog_pos{ 0.0f };
		float visible_distance = 4.0f;
		float base_opacity = 0.65f;
		float elapsed_time = 0.0f;
		float glow_speed = 0.35f;
		float glow_width = 0.09f;
		float glow_intensity = 1.15f;
	};

	static std::expected<dh::Pipeline, dh::GraphicsError> CreatePipeline(
		dh::RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera3d const & camera3d);

private:
	ScentTrailPipeline() = delete;
};

std::expected<dh::Pipeline, dh::GraphicsError> ScentTrailPipeline::CreatePipeline(
	dh::RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera3d const & camera3d)
{
	struct ObjectDataFS
	{
		alignas(16) glm::vec4 color{ 1.0f };
		alignas(8) glm::vec2 dog_pos{ 0.0f };
		alignas(4) float visible_distance = 4.0f;
		alignas(4) float base_opacity = 0.65f;
		alignas(4) float elapsed_time = 0.0f;
		alignas(4) float glow_speed = 0.35f;
		alignas(4) float glow_width = 0.09f;
		alignas(4) float glow_intensity = 1.15f;
	};

	dh::PipelineBuilder builder{ render_context };

	std::expected<void, dh::GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "scent_trail.vert",
		shaders_path / "scent_trail.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetObjectDataTypes<std::nullopt_t, ObjectDataFS>();
	builder.SetDepthTestOptions(dh::DepthTestOptions{
		.enable_depth_test = true,
		.enable_depth_write = false,
		.depth_compare_op = dh::DepthCompareOp::LESS_OR_EQUAL
		});
	builder.SetBlendOptions(dh::BlendOptions{
		.enable_blend = true,
		.src_factor = dh::BlendFactor::SRC_ALPHA,
		.dst_factor = dh::BlendFactor::ONE_MINUS_SRC_ALPHA
		});
	builder.SetCullMode(dh::CullMode::NONE);

	builder.SetPerFrameConstantsCallback(
		[&camera3d](dh::Pipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera3d.GetViewProjUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[](dh::Pipeline const & pipeline, void const * object_data)
		{
			if (!object_data)
			{
				LOG_FIRST_N(ERROR, 10) << "ScentTrailPipeline: ObjectData is null";
				return;
			}

			auto const * data = static_cast<ObjectData const *>(object_data);
			pipeline.SetObjectData(
				std::nullopt,
				ObjectDataFS{
					.color = data->color,
					.dog_pos = data->dog_pos,
					.visible_distance = data->visible_distance,
					.base_opacity = data->base_opacity,
					.elapsed_time = data->elapsed_time,
					.glow_speed = data->glow_speed,
					.glow_width = data->glow_width,
					.glow_intensity = data->glow_intensity
				});
		});

	return builder.CreatePipeline();
}
