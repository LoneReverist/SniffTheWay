// SceneHome.ixx

module;

export module SceneHome;

import Dreamhearth;

import StoryScene;

namespace dh = Dreamhearth;

export class SceneHome : public StoryScene
{
public:
	explicit SceneHome(dh::RenderContext const & render_context);
};

SceneHome::SceneHome(dh::RenderContext const & render_context)
	: StoryScene(render_context, "home")
{
}
