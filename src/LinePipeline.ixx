// LinePipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

export module LinePipeline;

import Dreamhearth;
using namespace Dreamhearth;

import Camera;
import Vertex;

export struct LineInstance
{
    glm::vec3 p0{ 0.0f };
    glm::vec3 p1{ 0.0f };
    float thickness = 0.0f;
    glm::vec4 color{ 0.0f };

    static LayoutDesc CreateLayout();
};

export class LinePipeline
{
public:
	using VertexT = Vertex2d;

	static std::expected<Pipeline, GraphicsError> CreatePipeline(
		RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera3d const & camera3d);

private:
	LinePipeline() = delete;
};

std::expected<Pipeline, GraphicsError> LinePipeline::CreatePipeline(
	RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera3d const & camera3d)
{
	PipelineBuilder builder{ render_context };

	std::expected<void, GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "line.vert",
		shaders_path / "line.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
    builder.SetInstanceType<LineInstance>();
	builder.SetVSUniformTypes<ViewProjUniform, ViewportUniform>();
	builder.SetBlendOptions(BlendOptions{
		.enable_blend = true,
		.src_factor = BlendFactor::SRC_ALPHA,
		.dst_factor = BlendFactor::ONE_MINUS_SRC_ALPHA
		});
	builder.SetCullMode(CullMode::NONE);

	builder.SetPerFrameConstantsCallback(
		[&camera3d](Pipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera3d.GetViewProjUniform());
            pipeline.SetUniform(1 /*binding*/, camera3d.GetViewportUniform());
		});

	return builder.CreatePipeline();
}

LayoutDesc LineInstance::CreateLayout()
{
    LayoutDesc layout;
	layout.binding = 1; // vertex uses binding 0, instance uses binding 1
	layout.stride = sizeof(LineInstance);
	layout.input_rate = InputRate::Instance;
    layout.attributes = {
        { AttributeType::Float3, offsetof(LineInstance, p0), 1 },
        { AttributeType::Float3, offsetof(LineInstance, p1), 2 },
        { AttributeType::Float, offsetof(LineInstance, thickness), 3 },
        { AttributeType::Float4, offsetof(LineInstance, color), 4 }
    };
	return layout;
}
