// SceneNight.ixx

module;

export module SceneNight;

import Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneNight : public StoryScene
{
public:
	explicit SceneNight(dh::RenderContext const & render_context);
};

SceneNight::SceneNight(dh::RenderContext const & render_context)
	: StoryScene(render_context, SceneId::Night)
{
}
