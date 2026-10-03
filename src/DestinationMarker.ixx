// DestinationMarker.ixx

module;

#include <cmath>
#include <optional>
#include <vector>

#include <glm/glm.hpp>


export module DestinationMarker;

import AssetManager;
import Camera;
import LinePipeline;
import SceneRenderer;
import RenderObject;
import SniffTheWayConstants;
import Vertex;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class DestinationMarker
{
public:
	void Init(AssetManager & asset_manager, SceneRenderer & renderer, Camera3d const & camera);
	void Update(AssetManager & asset_manager, SceneRenderer & renderer, std::optional<glm::vec2> destination);

private:
	MeshId<Vertex2d> m_mesh_id;
	AssetId m_render_object_id;
	std::optional<glm::vec2> m_destination;
};

void DestinationMarker::Init(AssetManager & asset_manager, SceneRenderer & renderer, Camera3d const & camera)
{
	auto const pipeline_id = asset_manager.AddPipeline<LinePipeline>(camera);
	m_mesh_id = asset_manager.AddMesh(
		std::vector<Vertex2d>{ {{-1, 1}}, {{1, 1}}, {{-1, -1}}, {{1, -1}} },
		std::vector<dh::Mesh::IndexT>{ 1, 0, 2, 1, 2, 3 },
		std::vector<LineInstance>{ LineInstance{} });
	m_render_object_id = renderer.CreateRenderObject(
		"movement destination", RenderLayer::Scene3dGroundShadow, m_mesh_id, pipeline_id);
	renderer.Show(m_render_object_id, false);
}

void DestinationMarker::Update(
	AssetManager & asset_manager, SceneRenderer & renderer, std::optional<glm::vec2> destination)
{
	renderer.Show(m_render_object_id, destination.has_value());
	if (destination && destination != m_destination)
	{
		std::vector<LineInstance> lines;
		constexpr int segments = 32;
		for (int i = 0; i < segments; ++i)
		{
			float const a = 6.2831853f * i / segments;
			float const b = 6.2831853f * (i + 1) / segments;
			lines.push_back(LineInstance{
				.p0 = { destination->x + 0.14f * std::cos(a), destination->y + 0.14f * std::sin(a), 0.015f },
				.p1 = { destination->x + 0.14f * std::cos(b), destination->y + 0.14f * std::sin(b), 0.015f },
				.thickness = 3.0f,
				.color = { 1.0f, 0.85f, 0.35f, 0.9f }
			});
		}
		auto const old_mesh = m_mesh_id;
		m_mesh_id = asset_manager.AddMesh(
			std::vector<Vertex2d>{ {{-1, 1}}, {{1, 1}}, {{-1, -1}}, {{1, -1}} },
			std::vector<dh::Mesh::IndexT>{ 1, 0, 2, 1, 2, 3 }, lines);
		if (auto * render_object = renderer.GetRenderObject(m_render_object_id))
			render_object->SetMeshId(m_mesh_id);
		asset_manager.RemoveMesh(old_mesh);
	}
	m_destination = destination;
}
