// SceneCreek.ixx

export module SceneCreek;

import Dreamhearth;
namespace dh = Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

using namespace SniffTheWay;

export class SceneCreek : public StoryScene
{
public:
	explicit SceneCreek(dh::RenderContext const & render_context);
};

SceneCreek::SceneCreek(dh::RenderContext const & render_context)
	: StoryScene(render_context, { "approaching_creek.png", "crossing_creek.png", "beyond_the_creek.png" })
{
	m_next_scene_id = SceneId::ForestIntersection;
}
