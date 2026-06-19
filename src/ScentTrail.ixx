// ScentTrail.ixx

module;

#include <algorithm>
#include <cmath>
#include <vector>

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

export module ScentTrail;

import Dreamhearth;

import AssetManager;
import AssetPool;
import GameplaySceneData;
import ScentTrailPipeline;
import Vertex;

namespace dh = Dreamhearth;

export class ScentTrail
{
public:
	void Init(AssetManager & asset_manager, ScentTrailData const & trail_data, glm::vec2 dog_pos);
	void Reload(AssetManager & asset_manager, ScentTrailData const & trail_data, glm::vec2 dog_pos);
	void Update(glm::vec2 dog_pos);

	bool IsValid() const { return m_mesh_id.IsValid(); }
	MeshId<ScentTrailVertex> GetMeshId() const { return m_mesh_id; }
	ScentTrailPipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }

private:
	struct Sample
	{
		glm::vec2 pos{ 0.0f };
		float distance = 0.0f;
	};

	MeshId<ScentTrailVertex> create_mesh(AssetManager & asset_manager, ScentTrailData const & trail_data) const;
	std::vector<Sample> sample_path(std::vector<glm::vec2> const & points) const;
	static glm::vec2 catmull_rom(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t);

private:
	MeshId<ScentTrailVertex> m_mesh_id;
	ScentTrailPipeline::ObjectData m_pipeline_data;
};

void ScentTrail::Init(AssetManager & asset_manager, ScentTrailData const & trail_data, glm::vec2 dog_pos)
{
	m_mesh_id = create_mesh(asset_manager, trail_data);
	m_pipeline_data.color = glm::vec4{ 1.0f, 0.84f, 0.42f, 1.0f };
	m_pipeline_data.dog_pos = dog_pos;
	m_pipeline_data.visible_distance = std::max(trail_data.visible_distance, 0.1f);
	m_pipeline_data.base_opacity = 0.68f;
}

void ScentTrail::Reload(AssetManager & asset_manager, ScentTrailData const & trail_data, glm::vec2 dog_pos)
{
	if (m_mesh_id.IsValid())
		asset_manager.RemoveMesh(m_mesh_id);

	Init(asset_manager, trail_data, dog_pos);
}

void ScentTrail::Update(glm::vec2 dog_pos)
{
	m_pipeline_data.dog_pos = dog_pos;
}

MeshId<ScentTrailVertex> ScentTrail::create_mesh(AssetManager & asset_manager, ScentTrailData const & trail_data) const
{
	std::vector<Sample> samples = sample_path(trail_data.points);
	if (samples.size() < 2)
		return {};

	const float half_width = std::max(trail_data.width, 0.01f) * 0.5f;
	const float total_distance = std::max(samples.back().distance, 0.001f);
	constexpr float ground_z = 0.012f;

	std::vector<ScentTrailVertex> verts;
	verts.reserve(samples.size() * 2);

	for (std::size_t i = 0; i < samples.size(); ++i)
	{
		glm::vec2 tangent{ 0.0f };
		if (i == 0)
			tangent = samples[i + 1].pos - samples[i].pos;
		else if (i + 1 == samples.size())
			tangent = samples[i].pos - samples[i - 1].pos;
		else
			tangent = samples[i + 1].pos - samples[i - 1].pos;

		if (glm::length(tangent) < 1e-5f)
			tangent = glm::vec2{ 0.0f, 1.0f };
		else
			tangent = glm::normalize(tangent);

		glm::vec2 normal{ -tangent.y, tangent.x };
		float const trail_t = samples[i].distance / total_distance;

		verts.push_back(ScentTrailVertex{
			.pos = glm::vec3{ samples[i].pos - normal * half_width, ground_z },
			.trail_t = trail_t,
			.side = -1.0f
		});
		verts.push_back(ScentTrailVertex{
			.pos = glm::vec3{ samples[i].pos + normal * half_width, ground_z },
			.trail_t = trail_t,
			.side = 1.0f
		});
	}

	std::vector<dh::Mesh::IndexT> indices;
	indices.reserve((samples.size() - 1) * 6);
	for (std::size_t i = 0; i + 1 < samples.size(); ++i)
	{
		const dh::Mesh::IndexT left0 = static_cast<dh::Mesh::IndexT>(i * 2);
		const dh::Mesh::IndexT right0 = left0 + 1;
		const dh::Mesh::IndexT left1 = left0 + 2;
		const dh::Mesh::IndexT right1 = left0 + 3;

		indices.insert(indices.end(), {
			left0, right0, left1,
			right0, right1, left1
		});
	}

	return asset_manager.AddMesh(verts, indices);
}

std::vector<ScentTrail::Sample> ScentTrail::sample_path(std::vector<glm::vec2> const & points) const
{
	std::vector<Sample> samples;
	if (points.size() < 2)
		return samples;

	constexpr int samples_per_segment = 12;
	samples.reserve((points.size() - 1) * samples_per_segment + 1);

	for (std::size_t i = 0; i + 1 < points.size(); ++i)
	{
		glm::vec2 const p0 = i == 0 ? points[i] : points[i - 1];
		glm::vec2 const p1 = points[i];
		glm::vec2 const p2 = points[i + 1];
		glm::vec2 const p3 = i + 2 < points.size() ? points[i + 2] : points[i + 1];

		for (int step = 0; step < samples_per_segment; ++step)
		{
			if (i > 0 && step == 0)
				continue;

			const float t = static_cast<float>(step) / static_cast<float>(samples_per_segment);
			Sample sample{ .pos = catmull_rom(p0, p1, p2, p3, t) };
			if (!samples.empty())
				sample.distance = samples.back().distance + glm::distance(samples.back().pos, sample.pos);
			samples.push_back(sample);
		}
	}

	Sample last_sample{ .pos = points.back() };
	if (!samples.empty())
		last_sample.distance = samples.back().distance + glm::distance(samples.back().pos, last_sample.pos);
	samples.push_back(last_sample);

	return samples;
}

glm::vec2 ScentTrail::catmull_rom(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t)
{
	const float t2 = t * t;
	const float t3 = t2 * t;
	return 0.5f * ((2.0f * p1)
		+ (-p0 + p2) * t
		+ (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2
		+ (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}
