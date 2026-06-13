// ScenePicnic.ixx

module;

export module ScenePicnic;

import Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class ScenePicnic : public StoryScene
{
public:
	explicit ScenePicnic(dh::RenderContext const & render_context);

private:
	static constexpr SceneId NextSceneId = SceneId::ForestPath;
};

ScenePicnic::ScenePicnic(dh::RenderContext const & render_context)
	: StoryScene(render_context, "picnic", NextSceneId)
{
}
