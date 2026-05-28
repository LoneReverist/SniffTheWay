// ScenePicnic.ixx

export module ScenePicnic;

import Dreamhearth;
namespace dh = Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

using namespace SniffTheWay;

export class ScenePicnic : public StoryScene
{
public:
	explicit ScenePicnic(dh::RenderContext const & render_context);
};

ScenePicnic::ScenePicnic(dh::RenderContext const & render_context)
	: StoryScene(render_context, { "picnic.png", "gust_of_wind.png", "following_butterflies.png", "lost.png" })
{
	m_next_scene_id = SceneId::ForestPath;
}
