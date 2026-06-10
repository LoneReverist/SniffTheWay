// BlurPipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>

#include <glm/vec2.hpp>

export module BlurPipeline;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Camera;
import Vertex;

namespace dh = Dreamhearth;

export class BlurPipeline
{
public:
	using VertexT = TextureVertex2d;

	struct ObjectData
	{
		AssetId tex_id;
		glm::vec2 texel_step{ 0.0f };
		int blur_radius{ 1 };
		float alpha_boost{ 1.0f };
	};

	static std::expected<dh::Pipeline, dh::GraphicsError> CreatePipeline(
		dh::RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera2d const & camera2d,
		AssetManager const & asset_manager);

private:
	BlurPipeline() = delete;
};

std::expected<dh::Pipeline, dh::GraphicsError> BlurPipeline::CreatePipeline(
	dh::RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera2d const & camera2d,
	AssetManager const & asset_manager)
{
	struct ObjectDataFS
	{
		alignas(8) glm::vec2 texel_step{ 0.0f };
		alignas(4) int blur_radius{ 1 };
		alignas(4) float alpha_boost{ 1.0f };
	};

	dh::PipelineBuilder builder{ render_context };

	std::expected<void, dh::GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "texture2d.vert",
		shaders_path / "blur.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetVSUniformTypes<ProjUniform>();
	builder.SetObjectDataTypes<std::nullopt_t, ObjectDataFS>();
	builder.SetHasTexture(true);
	builder.SetCullMode(dh::CullMode::NONE);
	builder.SetDepthTestOptions(dh::DepthTestOptions{
		.uses_depth_attachment = false, // render to texture with no depth buffer
		.enable_depth_test = false,
		.enable_depth_write = false,
		.depth_compare_op = dh::DepthCompareOp::ALWAYS
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
				std::cout << "BlurPipeline: ObjectData is null" << std::endl;
				return;
			}

			auto const * data = static_cast<ObjectData const *>(object_data);

			dh::Texture const * texture = asset_manager.GetTexture(data->tex_id);
			if (texture)
				pipeline.BindTexture(0, *texture);
			else
				std::cout << "BlurPipeline: No texture found in pool for AssetId: " << data->tex_id.GetIndex() << std::endl;

			pipeline.SetObjectData(
				std::nullopt,
				ObjectDataFS{
					.texel_step = data->texel_step,
					.blur_radius = data->blur_radius,
					.alpha_boost = data->alpha_boost
				});
		});

	return builder.CreatePipeline();
}
