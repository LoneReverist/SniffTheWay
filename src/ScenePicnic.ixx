// ScenePicnic.ixx

module;

export module ScenePicnic;

import Dreamhearth;

import StoryScene;

namespace dh = Dreamhearth;

export class ScenePicnic : public StoryScene
{
public:
	explicit ScenePicnic(dh::RenderContext const & render_context);
};

ScenePicnic::ScenePicnic(dh::RenderContext const & render_context)
	: StoryScene(render_context, "picnic")
{
}
