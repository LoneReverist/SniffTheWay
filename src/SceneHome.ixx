// SceneHome.ixx

module;

export module SceneHome;

import Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneHome : public StoryScene
{
public:
	explicit SceneHome(dh::RenderContext const & render_context);

private:
	static constexpr SceneId NextSceneId = SceneId::Exit;
};

SceneHome::SceneHome(dh::RenderContext const & render_context)
	: StoryScene(render_context, "home", NextSceneId)
{
}
