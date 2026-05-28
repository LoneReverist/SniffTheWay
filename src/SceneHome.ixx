// SceneHome.ixx

export module SceneHome;

import Dreamhearth;
namespace dh = Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

using namespace SniffTheWay;

export class SceneHome : public StoryScene
{
public:
	explicit SceneHome(dh::RenderContext const & render_context);
};

SceneHome::SceneHome(dh::RenderContext const & render_context)
	: StoryScene(render_context, { "home.png", "reunion.png", "safe_again.png" })
{
	m_next_scene_id = SceneId::Exit;
}
