// Scene.cpp

module;

#include <string>

#include <glm/gtc/matrix_transform.hpp>

module Scene;

import PlatformUtils;

Scene::Scene(RenderContext const & render_context, std::string const & title, float dpi_scale_factor)
	: m_render_context{ render_context }
	, m_resources_path{ PlatformUtils::GetExecutableDir() / "resources" }
	, m_title{ title }
	, m_renderer{ render_context }
{
}

void Scene::OnViewportResized(int width, int height)
{
}

void Scene::OnDPIScalingFactorChanged(float dpi_scale_factor)
{
}

bool Scene::Update(float dt, Input const & input)
{
	if (input.KeyIsPressed(Input::Key::Esc))
		return false;

	glm::vec3 bg_color = { 0.0f, 0.0f, 0.0f };
	m_renderer.SetClearColor(bg_color);

	return true;
}

void Scene::Render() const
{
	m_renderer.BeginDraw();

//	for (PipelineRenderObjects const & pipeline_r_objs : m_active_render_objects)
//	{
//		Pipeline const * pipeline = m_pipeline_pool.Get(pipeline_r_objs.pipeline_id);
//		if (!pipeline)
//		{
//			std::cout << "Scene::Render: No pipeline found in pool for pipeline ID: " << pipeline_r_objs.pipeline_id.GetIndex() << std::endl;
//			continue;
//		}
//
//		pipeline->Activate();
//		pipeline->UpdatePerFrameConstants();
//
//		for (AssetId obj_id : pipeline_r_objs.render_object_ids)
//		{
//			RenderObject const * obj = m_render_object_pool.Get(obj_id);
//			if (!obj)
//			{
//				std::cout << "Scene::Render: No render object found in pool for AssetId: " << obj_id.GetIndex() << std::endl;
//				continue;
//			}
//
//			Mesh const * mesh = m_mesh_manager.Get(obj->GetMeshId());
//			if (!mesh)
//			{
//				std::cout << "Scene::Render: No mesh found in pool for AssetId: " << obj->GetMeshId().GetIndex() << std::endl;
//				continue;
//			}
//
//			pipeline->UpdatePerObjectConstants(obj->GetObjectData());
//			mesh->Render();
//		}
//	}

	m_renderer.EndDraw();
}
