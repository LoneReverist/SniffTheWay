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

using namespace Dreamhearth;

export class SpritePipeline
{
public:
	using VertexT = TextureVertex2d;

	struct ObjectData
	{
		glm::mat4 model;
		glm::vec4 frame_uvs; // x = min_u, y = max_u, z = min_v, w = max_v
	};

	static std::expected<Pipeline, GraphicsError> CreatePipeline(
		RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera3d const & camera3d,
		AssetManager const & asset_manager,
		AssetId texture_id);

private:
	SpritePipeline() = delete;
};

std::expected<Pipeline, GraphicsError> SpritePipeline::CreatePipeline(
	RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera3d const & camera3d,
	AssetManager const & asset_manager,
	AssetId texture_id)
{
	struct ObjectDataVS
	{
		alignas(16) glm::mat4 model;
	};
	struct ObjectDataFS
	{
		alignas(16) glm::vec4 frame_uvs;
	};

	Texture const * texture = asset_manager.GetTexture(texture_id);
	if (!texture)
		return std::unexpected{ GraphicsError{ "SpritePipeline::CreatePipeline: invalid texture" } };

	PipelineBuilder builder{ render_context };

	std::expected<void, GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "sprite.vert",
		shaders_path / "sprite.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetObjectDataTypes<ObjectDataVS, ObjectDataFS>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetTexture(*texture);
	builder.SetBlendOptions(BlendOptions{
		.enable_blend = true,
		.src_factor = BlendFactor::SRC_ALPHA,
		.dst_factor = BlendFactor::ONE_MINUS_SRC_ALPHA
		});
	// disable backface culling since these are just flat quads that can be rotated
	// in any direction, and we don't want them to disappear when viewed from behind
	builder.SetCullMode(CullMode::NONE);

	builder.SetPerFrameConstantsCallback(
		[&camera3d](Pipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera3d.GetViewProjUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[](Pipeline const & pipeline, void const * object_data)
		{
			if (!object_data)
			{
				std::cout << "ObjectData is null for SpritePipeline" << std::endl;
				return;
			}

			auto const * data = static_cast<ObjectData const *>(object_data);

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
