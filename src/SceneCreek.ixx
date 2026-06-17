// SceneCreek.ixx

module;

export module SceneCreek;

import Dreamhearth;

import StoryScene;

namespace dh = Dreamhearth;

export class SceneCreek : public StoryScene
{
public:
	explicit SceneCreek(dh::RenderContext const & render_context);
};

SceneCreek::SceneCreek(dh::RenderContext const & render_context)
	: StoryScene(render_context, "creek")
{
}
