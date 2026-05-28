// SceneNight.ixx

export module SceneNight;

import Dreamhearth;
namespace dh = Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

using namespace SniffTheWay;

export class SceneNight : public StoryScene
{
public:
	explicit SceneNight(dh::RenderContext const & render_context);
};

SceneNight::SceneNight(dh::RenderContext const & render_context)
	: StoryScene(render_context, { "night.png", "morning.png" })
{
	m_next_scene_id = SceneId::ForestIntersection;
}
