module;

#include <algorithm>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glog/logging.h>

export module Squirrel;

import Dreamhearth;
import AssetManager;
import AssetPool;
import Camera;
import EnvironmentObject;
import EnvironmentObjectData;
import GameplaySceneData;
import SceneRenderer;
import RenderObject;
import SniffTheWayConstants;
import SpritePipeline;
import SquirrelData;
import Vertex;

// Nonmovable: the renderer retains the address of m_pipeline_data.
export class Squirrel
{
public:
	enum class State { Hidden, Bouncing, Fading, Gone };
	Squirrel() = default;
	Squirrel(Squirrel const &) = delete;
	Squirrel & operator=(Squirrel const &) = delete;
	void Init(SquirrelData const & data, bool found, AssetManager & assets, SceneRenderer & renderer,
		PipelineId<SpritePipeline> solid, PipelineId<SpritePipeline> translucent);
	void Destroy(AssetManager & assets, SceneRenderer & renderer);
	bool CanDiscover(glm::vec2 dog_position) const
	{
		return m_render_id.IsValid() && m_state == State::Hidden && m_data.discovery_region.Contains(dog_position);
	}
	void Reveal();
	void Update(float dt);
	// Editor presentation never changes m_state or m_elapsed (live discovery).
	void SetEditorSelection(bool selected, bool surprised);
	void PreviewReaction() { if (m_editor_selected) m_preview_elapsed = 0.0f; }
	void UpdateEditorPreview(float dt);
	void ApplyEditorData(SquirrelData const & data) { m_data = data; m_preview_elapsed = -1.0f; }
	void Refresh(GameplayCameraData const & authored, Camera3d const & camera,
		glm::vec3 scene_tint, bool editing, SceneRenderer & renderer);
	bool IsReady() const { return m_render_id.IsValid(); }
	bool IsTranslucent() const { return m_state != State::Hidden; }
	bool IsFound() const { return m_state != State::Hidden; }
	State GetState() const { return m_state; }
	std::string const & GetId() const { return m_data.id; }
	AssetId GetRenderObjectId() const { return m_render_id; }
	float GetDepth(Camera3d const & camera) const
	{
		return glm::dot(m_center - camera.GetPosition(), camera.GetDir());
	}
private:
	SquirrelData m_data;
	State m_state = State::Hidden;
	float m_elapsed = 0.0f;
	bool m_editor_selected = false;
	bool m_editor_surprised = false;
	float m_preview_elapsed = -1.0f;
	AssetId m_hidden_texture;
	AssetId m_surprised_texture;
	MeshId<TextureVertex2d> m_mesh;
	AssetId m_render_id;
	PipelineId<SpritePipeline> m_solid_pipeline;
	PipelineId<SpritePipeline> m_translucent_pipeline;
	SpritePipeline::ObjectData m_pipeline_data;
	glm::vec3 m_center{ 0.0f };
};

void Squirrel::Init(SquirrelData const & data, bool found, AssetManager & assets, SceneRenderer & renderer,
	PipelineId<SpritePipeline> solid, PipelineId<SpritePipeline> translucent)
{
	Destroy(assets, renderer);
	m_data = data;
	m_state = found ? State::Gone : State::Hidden;
	m_elapsed = 0;
	m_editor_selected = false;
	m_editor_surprised = false;
	m_preview_elapsed = -1;
	if (!data.IsValid())
	{
		LOG(WARNING) << "Invalid squirrel: " << data.id;
		return;
	}
	m_hidden_texture = assets.AddTexture(assets.GetTexturesPath() / data.hidden_pose.texture,
		Dreamhearth::PixelFormat::RGBA_SRGB, false, false);
	m_surprised_texture = assets.AddTexture(assets.GetTexturesPath() / data.surprised_pose.texture,
		Dreamhearth::PixelFormat::RGBA_SRGB, false, false);
	if (!m_hidden_texture.IsValid() || !m_surprised_texture.IsValid())
	{
		LOG(WARNING) << "Squirrel disabled because a pose texture could not load: " << data.id;
		Destroy(assets, renderer);
		return;
	}
	std::vector<TextureVertex2d> vertices{
		{ { 0, 0 }, { 0, 1 } }, { { 1, 0 }, { 1, 1 } },
		{ { 0, 1 }, { 0, 0 } }, { { 1, 1 }, { 1, 0 } } };
	std::vector<Dreamhearth::Mesh::IndexT> indices{ 0, 1, 2, 1, 3, 2 };
	m_mesh = assets.AddMesh(vertices, indices);
	m_solid_pipeline = solid;
	m_translucent_pipeline = translucent;
	m_pipeline_data.tex_id = m_hidden_texture;
	m_pipeline_data.frame_uvs = data.flip_horizontal ? glm::vec4{ 1, 0, 0, 1 } : glm::vec4{ 0, 1, 0, 1 };
	m_render_id = renderer.CreateRenderObject("squirrel " + data.id,
		SniffTheWay::RenderLayer::Scene3d, m_mesh, solid, m_pipeline_data);
	renderer.Show(m_render_id, !found);
}

void Squirrel::Destroy(AssetManager & assets, SceneRenderer & renderer)
{
	if (m_render_id.IsValid()) renderer.RemoveRenderObject(m_render_id);
	if (m_mesh.IsValid()) assets.RemoveMesh(m_mesh);
	if (m_hidden_texture.IsValid()) assets.RemoveTexture(m_hidden_texture);
	if (m_surprised_texture.IsValid()) assets.RemoveTexture(m_surprised_texture);
	m_render_id = {};
	m_mesh = {};
	m_hidden_texture = {};
	m_surprised_texture = {};
	m_pipeline_data = {};
}

void Squirrel::Reveal()
{
	if (!IsReady() || m_state != State::Hidden) return;
	m_state = State::Bouncing;
	m_elapsed = 0;
}

void Squirrel::Update(float dt)
{
	if (m_state == State::Hidden || m_state == State::Gone || !std::isfinite(dt)) return;
	m_elapsed += std::max(dt, 0.0f);
	if (m_elapsed >= m_data.bounce_duration + m_data.fade_duration)
		m_state = State::Gone;
	else if (m_elapsed >= m_data.bounce_duration)
		m_state = State::Fading;
}

void Squirrel::SetEditorSelection(bool selected, bool surprised)
{
	if (selected != m_editor_selected || surprised != m_editor_surprised || !selected)
		m_preview_elapsed = -1.0f;
	m_editor_selected = selected;
	m_editor_surprised = surprised;
}

void Squirrel::UpdateEditorPreview(float dt)
{
	if (m_preview_elapsed < 0.0f || !std::isfinite(dt)) return;
	m_preview_elapsed += std::max(dt, 0.0f);
	if (m_preview_elapsed >= m_data.bounce_duration + m_data.fade_duration)
		m_preview_elapsed = -1.0f;
}

void Squirrel::Refresh(GameplayCameraData const & authored, Camera3d const & camera,
	glm::vec3 scene_tint, bool editing, SceneRenderer & renderer)
{
	if (!IsReady()) return;
	State display_state = m_state;
	float elapsed = m_elapsed;
	if (editing)
	{
		elapsed = std::max(m_preview_elapsed, 0.0f);
		display_state = m_preview_elapsed >= 0.0f
			? (elapsed < m_data.bounce_duration ? State::Bouncing : State::Fading)
			: m_editor_selected && m_editor_surprised ? State::Bouncing : State::Hidden;
	}
	bool const hidden = display_state == State::Hidden;
	SquirrelPoseData const & pose = hidden ? m_data.hidden_pose : m_data.surprised_pose;
	EnvironmentObjectData placement;
	placement.position = m_data.position + pose.offset;
	placement.size = pose.size;
	m_pipeline_data.model = EnvironmentObject::CalculateTransform(placement, authored, camera);
	float opacity = 1.0f;
	if (display_state == State::Bouncing)
	{
		float const u = glm::clamp(elapsed / m_data.bounce_duration, 0.0f, 1.0f);
		glm::vec3 const up = glm::normalize(glm::vec3{ m_pipeline_data.model[1] });
		m_pipeline_data.model[3] += glm::vec4{ up * (4.0f * m_data.bounce_height * u * (1.0f - u)), 0 };
	}
	if (display_state == State::Fading)
		opacity = 1.0f - glm::smoothstep(0.0f, 1.0f, (elapsed - m_data.bounce_duration) / m_data.fade_duration);
	m_pipeline_data.tex_id = hidden ? m_hidden_texture : m_surprised_texture;
	// Recompute from source colors; opacity updates must never overwrite scene RGB.
	m_pipeline_data.tint = glm::vec4{ scene_tint * glm::vec3{ m_data.tint },
		m_data.tint.a * pose.opacity * opacity * (editing && !m_editor_selected ? 0.3f : 1.0f) };
	m_center = glm::vec3{ m_pipeline_data.model * glm::vec4{ 0.5f, 0.5f, 0, 1 } };
	if (auto * object = renderer.GetRenderObject(m_render_id))
	{
		object->SetPipelineId(editing || !hidden ? m_translucent_pipeline : m_solid_pipeline);
		object->Show(editing || m_state != State::Gone);
	}
}
