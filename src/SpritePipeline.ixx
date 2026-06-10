// SpritePipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

export module SpritePipeline;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Camera;
import Vertex;

namespace dh = Dreamhearth;

export class SpritePipeline
{
public:
	using VertexT = TextureVertex2d;

	struct ObjectData
	{
		glm::mat4 model{ 1.0f };
		glm::vec4 frame_uvs{ 0.0f }; // x = min_u, y = max_u, z = min_v, w = max_v
		AssetId tex_id;
	};

	static std::expected<dh::Pipeline, dh::GraphicsError> CreatePipeline(
		dh::RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera3d const & camera3d,
		AssetManager const & asset_manager);

private:
	SpritePipeline() = delete;
};

std::expected<dh::Pipeline, dh::GraphicsError> SpritePipeline::CreatePipeline(
	dh::RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera3d const & camera3d,
	AssetManager const & asset_manager)
{
	struct ObjectDataVS
	{
		alignas(16) glm::mat4 model{ 1.0f };
	};
	struct ObjectDataFS
	{
		alignas(16) glm::vec4 frame_uvs{ 0.0f };
	};

	dh::PipelineBuilder builder{ render_context };

	std::expected<void, dh::GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "sprite.vert",
		shaders_path / "sprite.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetObjectDataTypes<ObjectDataVS, ObjectDataFS>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetHasTexture(true);
	builder.SetBlendOptions(dh::BlendOptions{
		.enable_blend = true,
		.src_factor = dh::BlendFactor::SRC_ALPHA,
		.dst_factor = dh::BlendFactor::ONE_MINUS_SRC_ALPHA
		});
	// disable backface culling since these are just flat quads that can be rotated
	// in any direction, and we don't want them to disappear when viewed from behind
	builder.SetCullMode(dh::CullMode::NONE);

	builder.SetPerFrameConstantsCallback(
		[&camera3d](dh::Pipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera3d.GetViewProjUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[&asset_manager](dh::Pipeline const & pipeline, void const * object_data)
		{
			if (!object_data)
			{
				std::cout << "SpritePipeline: ObjectData is null" << std::endl;
				return;
			}

			auto const * data = static_cast<ObjectData const *>(object_data);

			dh::Texture const * texture = asset_manager.GetTexture(data->tex_id);
			if (texture)
				pipeline.BindTexture(0, *texture);
			else
				std::cout << "SpritePipeline: No texture found in pool for AssetId: " << data->tex_id.GetIndex() << std::endl;

			pipeline.SetObjectData(
				ObjectDataVS{
					.model = data->model
				},
				ObjectDataFS{
					.frame_uvs = data->frame_uvs
				});
		});

	return builder.CreatePipeline();
}
