// SceneRenderer.ixx

module;

#include <iostream>
#include <optional>
#include <string>

#include <glm/vec3.hpp>

export module SceneRenderer;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetPool;
import AssetManager;
import RenderObject;

export class SceneRenderer
{
public:
	explicit SceneRenderer(dh::RenderContext const & render_context, AssetManager const & asset_manager);

	template <typename MeshIdT, typename PipelineIdT, typename ObjectDataT = std::nullopt_t>
	requires MeshIsCompatibleWithPipeline<MeshIdT, PipelineIdT> && ObjectDataIsCompatibleWithPipeline<ObjectDataT, PipelineIdT>
	AssetId CreateRenderObject(
		std::string const & name,
		MeshIdT mesh_id,
		PipelineIdT pipeline_id,
		ObjectDataT const & object_data = std::nullopt);

	RenderObject * GetRenderObject(AssetId ro_id) { return m_render_object_pool.Get(ro_id); }

	void Show(AssetId ro_id, bool show);
	bool IsShown(AssetId ro_id) const;

	void Render() const;

private:
	struct RenderBatch
	{
		AssetId pipeline_id;
		std::vector<AssetId> ro_ids;
	};

private:
	dh::Renderer m_renderer;
	AssetManager const & m_asset_manager;
	AssetPool<RenderObject> m_render_object_pool;
	std::vector<RenderBatch> m_render_batches;
};

SceneRenderer::SceneRenderer(dh::RenderContext const & render_context, AssetManager const & asset_manager)
	: m_renderer(render_context)
	, m_asset_manager(asset_manager)
{
	m_renderer.SetClearColor(glm::vec3{ 0.0f, 0.0f, 0.0f });
}

template <typename MeshIdT, typename PipelineIdT, typename ObjectDataT /*= std::nullopt_t*/>
	requires MeshIsCompatibleWithPipeline<MeshIdT, PipelineIdT> && ObjectDataIsCompatibleWithPipeline<ObjectDataT, PipelineIdT>
AssetId SceneRenderer::CreateRenderObject(
	std::string const & name,
	MeshIdT mesh_id,
	PipelineIdT pipeline_id,
	ObjectDataT const & object_data /*= std::nullopt*/)
{
	if (!mesh_id.IsValid())
	{
		std::cout << "SceneRenderer::CreateRenderObject: Invalid mesh id for object: " + name;
		return AssetId{};
	}
	if (!pipeline_id.IsValid())
	{
		std::cout << "SceneRenderer::CreateRenderObject: Invalid pipeline id for object: " + name;
		return AssetId{};
	}

	RenderObject ro{ name, mesh_id, pipeline_id };
	if constexpr (!std::same_as<ObjectDataT, std::nullopt_t>)
		ro.SetObjectData(&object_data);

	AssetId ro_id = m_render_object_pool.Add(std::move(ro));
	if (!ro_id.IsValid())
	{
		std::cout << "SceneRenderer::CreateRenderObject: Failed to add render object to pool for object: " + name;
		return ro_id;
	}

	auto iter = std::ranges::find(m_render_batches, pipeline_id, &RenderBatch::pipeline_id);
	if (iter != m_render_batches.end())
		iter->ro_ids.push_back(ro_id);
	else
		m_render_batches.push_back(RenderBatch{ pipeline_id, { ro_id } });

	return ro_id;
}

void SceneRenderer::Show(AssetId ro_id, bool show)
{
	RenderObject * ro = m_render_object_pool.Get(ro_id);
	if (!ro)
		return;

	ro->Show(show);
}

bool SceneRenderer::IsShown(AssetId ro_id) const
{
	RenderObject const * ro = m_render_object_pool.Get(ro_id);
	return ro && ro->IsShown();
}

void SceneRenderer::Render() const
{
	m_renderer.BeginDraw();

	for (RenderBatch const & batch : m_render_batches)
	{
		dh::Pipeline const * pipeline = m_asset_manager.GetPipeline(batch.pipeline_id);
		if (!pipeline)
		{
			std::cout << "SceneRenderer::Render: No pipeline found in pool for pipeline ID: " << batch.pipeline_id.GetIndex() << std::endl;
			continue;
		}

		pipeline->UpdatePerFrameConstants();
		pipeline->Activate();

		for (AssetId ro_id : batch.ro_ids)
		{
			RenderObject const * ro = m_render_object_pool.Get(ro_id);
			if (!ro)
			{
				std::cout << "SceneRenderer::Render: No render object found in pool for AssetId: " << ro_id.GetIndex() << std::endl;
				continue;
			}

			if (!ro->IsShown())
				continue;

			dh::Mesh const * mesh = m_asset_manager.GetMesh(ro->GetMeshId());
			if (!mesh)
			{
				std::cout << "SceneRenderer::Render: No mesh found in pool for AssetId: " << ro->GetMeshId().GetIndex() << std::endl;
				continue;
			}

			pipeline->UpdatePerObjectConstants(ro->GetObjectData());
			mesh->Render();
		}
	}

	m_renderer.EndDraw();
}
