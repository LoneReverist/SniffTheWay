// LinePipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

export module LinePipeline;

import Dreamhearth;

import Camera;
import Vertex;

namespace dh = Dreamhearth;

export struct LineInstance
{
	glm::vec3 p0{ 0.0f };
	glm::vec3 p1{ 0.0f };
	float thickness = 0.0f;
	glm::vec4 color{ 0.0f };

	static dh::LayoutDesc CreateLayout();
};

export class LinePipeline
{
public:
	using VertexT = Vertex2d;

	static std::expected<dh::Pipeline, dh::GraphicsError> CreatePipeline(
		dh::RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera3d const & camera3d);

private:
	LinePipeline() = delete;
};

std::expected<dh::Pipeline, dh::GraphicsError> LinePipeline::CreatePipeline(
	dh::RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera3d const & camera3d)
{
	dh::PipelineBuilder builder{ render_context };

	std::expected<void, dh::GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "line.vert",
		shaders_path / "line.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
    builder.SetInstanceType<LineInstance>();
	builder.SetVSUniformTypes<ViewProjUniform, ViewportUniform>();
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
            pipeline.SetUniform(1 /*binding*/, camera3d.GetViewportUniform());
		});

	return builder.CreatePipeline();
}

dh::LayoutDesc LineInstance::CreateLayout()
{
    dh::LayoutDesc layout;
	layout.binding = 1; // vertex uses binding 0, instance uses binding 1
	layout.stride = sizeof(LineInstance);
	layout.input_rate = dh::InputRate::Instance;
    layout.attributes = {
        { dh::AttributeType::Float3, offsetof(LineInstance, p0), 1 },
        { dh::AttributeType::Float3, offsetof(LineInstance, p1), 2 },
        { dh::AttributeType::Float, offsetof(LineInstance, thickness), 3 },
        { dh::AttributeType::Float4, offsetof(LineInstance, color), 4 }
    };
	return layout;
}
