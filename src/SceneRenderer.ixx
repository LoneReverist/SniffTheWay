// SceneRenderer.ixx

module;

#include <algorithm>
#include <optional>
#include <span>
#include <string>
#include <array>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glog/logging.h>

export module SceneRenderer;

import Dreamhearth;

import AssetPool;
import AssetManager;
import RenderObject;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneRenderer
{
public:
	explicit SceneRenderer(dh::RenderContext const & render_context, AssetManager const & asset_manager);

	void SetViewport(int x, int y, int width, int height);

	template <typename MeshIdT, typename PipelineIdT, typename ObjectDataT = std::nullopt_t>
	requires MeshIsCompatibleWithPipeline<MeshIdT, PipelineIdT> && ObjectDataIsCompatibleWithPipeline<ObjectDataT, PipelineIdT>
	AssetId CreateRenderObject(
		std::string const & name,
		RenderLayer layer,
		MeshIdT mesh_id,
		PipelineIdT pipeline_id,
		ObjectDataT const & object_data = std::nullopt);
	void RemoveRenderObject(AssetId ro_id);
	void SetRenderObjectBefore(RenderLayer layer, AssetId ro_id, AssetId before_ro_id);

	RenderObject * GetRenderObject(AssetId ro_id) { return m_render_object_pool.Get(ro_id); }

	void Show(AssetId ro_id, bool show);
	bool IsShown(AssetId ro_id) const;

	void Render() const;
	void RenderToTexture(dh::Texture const & target, std::span<RenderObject const> render_objects, glm::vec4 const & clear_color) const;

private:
	void render_objects() const;
	void render_object(RenderObject const & ro) const;

private:
	dh::Renderer m_renderer;
	AssetManager const & m_asset_manager;
	AssetPool<RenderObject> m_render_object_pool;
	std::array<std::vector<AssetId>, static_cast<std::size_t>(RenderLayer::Count)> m_render_layers;
};

SceneRenderer::SceneRenderer(dh::RenderContext const & render_context, AssetManager const & asset_manager)
	: m_renderer(render_context)
	, m_asset_manager(asset_manager)
{
	m_renderer.SetClearColor(glm::vec3{ 0.0f, 0.0f, 0.0f });
}

void SceneRenderer::SetViewport(int x, int y, int width, int height)
{
	m_renderer.SetViewport(x, y, width, height);
}

template <typename MeshIdT, typename PipelineIdT, typename ObjectDataT /*= std::nullopt_t*/>
	requires MeshIsCompatibleWithPipeline<MeshIdT, PipelineIdT> && ObjectDataIsCompatibleWithPipeline<ObjectDataT, PipelineIdT>
AssetId SceneRenderer::CreateRenderObject(
	std::string const & name,
	RenderLayer layer,
	MeshIdT mesh_id,
	PipelineIdT pipeline_id,
	ObjectDataT const & object_data /*= std::nullopt*/)
{
	if (!mesh_id.IsValid())
	{
		LOG(ERROR) << "SceneRenderer::CreateRenderObject: Invalid mesh id for object: " << name;
		return AssetId{};
	}
	if (!pipeline_id.IsValid())
	{
		LOG(ERROR) << "SceneRenderer::CreateRenderObject: Invalid pipeline id for object: " << name;
		return AssetId{};
	}

	RenderObject ro{ name, mesh_id, pipeline_id };
	if constexpr (!std::same_as<ObjectDataT, std::nullopt_t>)
		ro.SetObjectData(&object_data);

	AssetId ro_id = m_render_object_pool.Add(std::move(ro));
	if (!ro_id.IsValid())
	{
		LOG(ERROR) << "SceneRenderer::CreateRenderObject: Failed to add render object to pool for object: " << name;
		return ro_id;
	}

	m_render_layers[static_cast<std::size_t>(layer)].push_back(ro_id);

	return ro_id;
}

void SceneRenderer::RemoveRenderObject(AssetId ro_id)
{
	for (std::vector<AssetId> & layer_ro_ids : m_render_layers)
		std::erase(layer_ro_ids, ro_id);
	m_render_object_pool.Remove(ro_id);
}

void SceneRenderer::SetRenderObjectBefore(RenderLayer layer, AssetId ro_id, AssetId before_ro_id)
{
	if (ro_id == before_ro_id)
		return;

	std::vector<AssetId> & layer_ro_ids = m_render_layers[static_cast<std::size_t>(layer)];
	auto const ro_iter = std::ranges::find(layer_ro_ids, ro_id);
	auto const before_ro_iter = std::ranges::find(layer_ro_ids, before_ro_id);
	if (ro_iter == layer_ro_ids.end() || before_ro_iter == layer_ro_ids.end() || ro_iter < before_ro_iter)
		return;

	std::rotate(before_ro_iter, ro_iter, std::next(ro_iter));
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
	render_objects();
	m_renderer.EndDraw();
}

void SceneRenderer::RenderToTexture(dh::Texture const & target, std::span<RenderObject const> render_objects, glm::vec4 const & clear_color) const
{
	m_renderer.BeginTextureDraw(target, clear_color);
	for (RenderObject const & ro : render_objects)
		render_object(ro);
	m_renderer.EndTextureDraw(target);
}

void SceneRenderer::render_objects() const
{
	for (std::vector<AssetId> const & layer_ro_ids : m_render_layers)
	{
		for (AssetId ro_id : layer_ro_ids)
		{
			RenderObject const * ro = m_render_object_pool.Get(ro_id);
			if (!ro)
			{
				LOG_FIRST_N(ERROR, 10) << "SceneRenderer::render_objects: No render object found in pool for AssetId: "
					<< ro_id.GetIndex();
				continue;
			}

			render_object(*ro);
		}
	}
}

void SceneRenderer::render_object(RenderObject const & ro) const
{
	if (!ro.IsShown())
		return;

	dh::Pipeline const * pipeline = m_asset_manager.GetPipeline(ro.GetPipelineId());
	if (!pipeline)
	{
		LOG_FIRST_N(ERROR, 10) << "SceneRenderer::render_object: No pipeline found in pool for pipeline ID: "
			<< ro.GetPipelineId().GetIndex();
		return;
	}

	dh::Mesh const * mesh = m_asset_manager.GetMesh(ro.GetMeshId());
	if (!mesh)
	{
		LOG_FIRST_N(ERROR, 10) << "SceneRenderer::render_object: No mesh found in pool for AssetId: "
			<< ro.GetMeshId().GetIndex();
		return;
	}

	pipeline->UpdatePerFrameConstants();
	pipeline->Activate();
	pipeline->UpdatePerObjectConstants(ro.GetObjectData());
	mesh->Render();
}
