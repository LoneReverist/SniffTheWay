// SceneFadeOverlay.ixx

module;

#include <algorithm>

export module SceneFadeOverlay;

import AssetManager;
import AssetPool;
import Camera;
import FadeOverlayPipeline;
import SceneRenderer;
import SniffTheWayConstants;
import UIDarkBackdrop;

using namespace SniffTheWay;

export class SceneFadeOverlay
{
public:
	void Init(AssetManager & asset_manager, SceneRenderer & renderer, Camera2d const & camera2d);
	void SetOpacity(float opacity);

private:
	UIDarkBackdrop m_backdrop;
	FadeOverlayPipeline::ObjectData m_pipeline_data;
};

void SceneFadeOverlay::Init(AssetManager & asset_manager, SceneRenderer & renderer, Camera2d const & camera2d)
{
	const auto pipeline_id = asset_manager.AddPipeline<FadeOverlayPipeline>(camera2d);
	m_backdrop.Init(asset_manager, 0.0f, UIWidth, 0.0f, UIHeight, 1.0f, 1.0f);
	m_backdrop.SetROId(renderer.CreateRenderObject(
		"scene fade overlay",
		RenderLayer::OverlayForeground,
		m_backdrop.GetMeshId(),
		pipeline_id,
		m_pipeline_data));
}

void SceneFadeOverlay::SetOpacity(float opacity)
{
	m_pipeline_data.color.a = std::clamp(opacity, 0.0f, 1.0f);
}

