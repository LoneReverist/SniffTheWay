// Texture2dPipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

export module Texture2dPipeline;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Camera;
import Vertex;

namespace dh = Dreamhearth;

export class Texture2dPipeline
{
public:
	using VertexT = TextureVertex2d;

	enum class ColorMode : int
	{
		Textured = 0,
		ColorMask = 1,
	};

	struct ObjectData
	{
		AssetId tex_id;
		glm::vec4 color{ 1.0f };
		ColorMode color_mode = ColorMode::Textured;
	};

	static std::expected<dh::Pipeline, dh::GraphicsError> CreatePipeline(
		dh::RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera2d const & camera2d,
		AssetManager const & asset_manager);

private:
	Texture2dPipeline() = delete;
};

std::expected<dh::Pipeline, dh::GraphicsError> Texture2dPipeline::CreatePipeline(
	dh::RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera2d const & camera2d,
	AssetManager const & asset_manager)
{
	struct ObjectDataFS
	{
		alignas(16) glm::vec4 color{ 1.0f };
		alignas(4) int color_mode = static_cast<int>(ColorMode::Textured);
	};

	dh::PipelineBuilder builder{ render_context };

	std::expected<void, dh::GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "texture2d.vert",
		shaders_path / "texture2d.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetVSUniformTypes<ProjUniform>();
	builder.SetObjectDataTypes<std::nullopt_t, ObjectDataFS>();
	builder.SetHasTexture(true);
	builder.SetCullMode(dh::CullMode::NONE);
	builder.SetDepthTestOptions(dh::DepthTestOptions{
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
				std::cout << "Texture2dPipeline: ObjectData is null" << std::endl;
				return;
			}

			auto const * data = static_cast<ObjectData const *>(object_data);

			dh::Texture const * texture = asset_manager.GetTexture(data->tex_id);
			if (texture)
				pipeline.BindTexture(0, *texture);
			else
				std::cout << "Texture2dPipeline: No texture found in pool for AssetId: " << data->tex_id.GetIndex() << std::endl;

			pipeline.SetObjectData(
				std::nullopt,
				ObjectDataFS{
					.color = data->color,
					.color_mode = static_cast<int>(data->color_mode),
				});
		});

	return builder.CreatePipeline();
}
