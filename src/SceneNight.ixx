// SceneNight.ixx

module;

export module SceneNight;

import Dreamhearth;

import StoryScene;

namespace dh = Dreamhearth;

export class SceneNight : public StoryScene
{
public:
	explicit SceneNight(dh::RenderContext const & render_context);
};

SceneNight::SceneNight(dh::RenderContext const & render_context)
	: StoryScene(render_context, "night")
{
}
