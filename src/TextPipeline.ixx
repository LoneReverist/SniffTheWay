// TextPipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/vec4.hpp>

export module TextPipeline;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Camera;
import RenderObject;
import Vertex;

namespace dh = Dreamhearth;

export class TextPipeline
{
public:
	using VertexT = TextureVertex2d;

	struct ObjectData
	{
		float screen_px_range = 1.0f;
		glm::vec4 bg_color = glm::vec4(0.0f);
		glm::vec4 text_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		AssetId tex_id;
	};

	static std::expected<dh::Pipeline, dh::GraphicsError> CreatePipeline(
		dh::RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera2d const & camera2d,
		AssetManager const & asset_manager);

private:
	TextPipeline() = delete;
};

std::expected<dh::Pipeline, dh::GraphicsError> TextPipeline::CreatePipeline(
	dh::RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera2d const & camera2d,
	AssetManager const & asset_manager)
{
	struct ObjectDataFS
	{
		alignas(4) float screen_px_range = 0.0f;
		alignas(16) glm::vec4 bg_color{ 0.0f };
		alignas(16) glm::vec4 text_color{ 0.0f };
	};

	dh::PipelineBuilder builder{ render_context };

	std::expected<void, dh::GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "msdf_text.vert",
		shaders_path / "msdf_text.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetVSUniformTypes<ProjUniform>();
	builder.SetObjectDataTypes<std::nullopt_t, ObjectDataFS>();
	builder.SetHasTexture(true);
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
	builder.SetCullMode(dh::CullMode::NONE);

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
				std::cout << "TextPipeline: ObjectData is null" << std::endl;
				return;
			}

			// For performance, we assume that the object data is of the correct type.
			// Safety is managed with compile-time checks when creating render objects to ensure the data is compatible with the pipeline.
			auto const * data = static_cast<ObjectData const *>(object_data);

			dh::Texture const * texture = asset_manager.GetTexture(data->tex_id);
			if (texture)
				pipeline.BindTexture(0, *texture);
			else
				std::cout << "TextPipeline: No texture found in pool for AssetId: " << data->tex_id.GetIndex() << std::endl;

			pipeline.SetObjectData(
				std::nullopt,
				ObjectDataFS{
					.screen_px_range = data->screen_px_range,
					.bg_color = data->bg_color,
					.text_color = data->text_color
				});
		});

	return builder.CreatePipeline();
}
