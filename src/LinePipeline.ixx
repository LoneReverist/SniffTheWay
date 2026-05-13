// LinePipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

export module LinePipeline;

import Dreamhearth;

import AssetPool;
import Camera;
import Vertex;

using namespace Dreamhearth;

export struct LineInstance
{
    glm::vec3 p0;
    glm::vec3 p1;
    float thickness;
    glm::vec4 color;

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

	LinePipeline() = default;
	explicit LinePipeline(AssetId asset_id) : m_asset_id(asset_id) {}

	AssetId GetAssetId() const { return m_asset_id; }

private:
	AssetId m_asset_id;
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
