module;
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glog/logging.h>

export module EnvironmentObject;
import Dreamhearth;
import AssetManager;
import AssetPool;
import Camera;
import EnvironmentObjectData;
import GameplaySceneData;
import SceneRenderer;
import SniffTheWayConstants;
import SpritePipeline;
import Vertex;

// Nonmovable: SceneRenderer retains a pointer to m_pipeline_data.
export class EnvironmentObject
{
public:
	EnvironmentObject() = default;
	EnvironmentObject(EnvironmentObject const &) = delete;
	EnvironmentObject & operator=(EnvironmentObject const &) = delete;
	void Init(EnvironmentObjectData const & data, AssetManager & assets, SceneRenderer & renderer,
		PipelineId<SpritePipeline> pipeline, GameplayCameraData const & authored, Camera3d const & camera);
	void Destroy(AssetManager & assets, SceneRenderer & renderer);
	void UpdateTransform(GameplayCameraData const & authored, Camera3d const & camera);
	void SetOpacity(float opacity) { m_pipeline_data.tint.a = m_data.tint.a * opacity; }
	static glm::mat4 CalculateTransform(EnvironmentObjectData const & data,
		GameplayCameraData const & authored, Camera3d const & camera);
	AssetId GetRenderObjectId() const { return m_render_id; }
	float GetDepth(Camera3d const & camera) const
	{
		return glm::dot(m_center - camera.GetPosition(), camera.GetDir());
	}
private:
	EnvironmentObjectData m_data;
	SpritePipeline::ObjectData m_pipeline_data;
	MeshId<TextureVertex2d> m_mesh;
	AssetId m_render_id;
	glm::vec3 m_center{ 0.0f };
};

void EnvironmentObject::Init(EnvironmentObjectData const & data, AssetManager & assets,
	SceneRenderer & renderer, PipelineId<SpritePipeline> pipeline,
	GameplayCameraData const & authored, Camera3d const & camera)
{
	Destroy(assets, renderer);
	m_data = data;
	auto finite = [](auto value) {
		for (int i = 0; i < value.length(); ++i)
			if (!std::isfinite(value[i])) return false;
		return true;
	};
	if (data.texture.empty() || !finite(data.position) || !finite(data.size) || !finite(data.anchor)
		|| !finite(data.image_rect) || !finite(data.tint)
		|| data.size.x <= 0.0f || data.size.y <= 0.0f
		|| data.image_rect.z <= 0.0f || data.image_rect.w <= 0.0f
		|| (data.placement == EnvironmentPlacement::Background
			&& (!std::isfinite(data.depth) || data.depth <= 0.1f || data.depth >= 100.0f)))
	{
		LOG(WARNING) << "Invalid environment object: " << data.id;
		return;
	}
	m_pipeline_data.tex_id = assets.AddTexture(assets.GetTexturesPath() / data.texture,
		Dreamhearth::PixelFormat::RGBA_SRGB, false, false);
	if (!m_pipeline_data.tex_id.IsValid()) return;
	std::vector<TextureVertex2d> vertices{
		{ { 0, 0 }, { 0, 1 } }, { { 1, 0 }, { 1, 1 } },
		{ { 0, 1 }, { 0, 0 } }, { { 1, 1 }, { 1, 0 } } };
	std::vector<Dreamhearth::Mesh::IndexT> indices{ 0, 1, 2, 1, 3, 2 };
	m_mesh = assets.AddMesh(vertices, indices);
	m_pipeline_data.frame_uvs = glm::vec4{ 0, 1, 0, 1 };
	m_pipeline_data.tint = data.tint;
	UpdateTransform(authored, camera);
	m_render_id = renderer.CreateRenderObject("environment " + data.id,
		SniffTheWay::RenderLayer::Scene3d, m_mesh, pipeline, m_pipeline_data);
}

void EnvironmentObject::Destroy(AssetManager & assets, SceneRenderer & renderer)
{
	if (m_render_id.IsValid()) renderer.RemoveRenderObject(m_render_id);
	if (m_mesh.IsValid()) assets.RemoveMesh(m_mesh);
	if (m_pipeline_data.tex_id.IsValid()) assets.RemoveTexture(m_pipeline_data.tex_id);
	m_render_id = AssetId{};
	m_mesh = MeshId<TextureVertex2d>{};
	m_pipeline_data = SpritePipeline::ObjectData{};
}

void EnvironmentObject::UpdateTransform(GameplayCameraData const & authored, Camera3d const & camera)
{
	m_pipeline_data.model = CalculateTransform(m_data, authored, camera);
	m_center = glm::vec3{ m_pipeline_data.model * glm::vec4{ 0.5f, 0.5f, 0, 1 } };
}

glm::mat4 EnvironmentObject::CalculateTransform(EnvironmentObjectData const & data,
	GameplayCameraData const & authored, Camera3d const & camera)
{
	bool const background = data.placement == EnvironmentPlacement::Background;
	glm::vec3 forward = background ? authored.direction : camera.GetDir();
	if (glm::length(forward) < 1e-6f) return glm::mat4{ 1.0f };
	forward = glm::normalize(forward);
	glm::vec3 right = glm::cross(forward, glm::vec3{ 0, 0, 1 });
	if (glm::length(right) < 1e-6f) right = glm::cross(forward, glm::vec3{ 0, 1, 0 });
	right = glm::normalize(right);
	glm::vec3 const up = glm::cross(right, forward);
	glm::vec3 horizontal = right * data.size.x;
	glm::vec3 vertical = up * data.size.y;
	glm::vec3 origin = data.position - horizontal * data.anchor.x - vertical * data.anchor.y;
	if (background)
	{
		float const height = 2.0f * data.depth * std::tan(glm::radians(authored.fov_degrees) * 0.5f);
		auto const viewport = camera.GetViewportUniform().size;
		float const aspect = viewport.y > 0 ? viewport.x / viewport.y
			: SniffTheWay::UIWidth / SniffTheWay::UIHeight;
		auto const rect = data.image_rect;
		origin = authored.position + forward * data.depth
			+ right * height * aspect * (rect.x - 0.5f)
			+ up * height * (0.5f - rect.y - rect.w);
		horizontal = right * height * aspect * rect.z;
		vertical = up * height * rect.w;
	}
	glm::mat4 model{ 1.0f };
	model[0] = glm::vec4{ horizontal, 0 };
	model[1] = glm::vec4{ vertical, 0 };
	model[2] = glm::vec4{ -forward, 0 };
	model[3] = glm::vec4{ origin, 1 };
	return model;
}
