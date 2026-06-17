// ScenePlayground.ixx

module;

export module ScenePlayground;

import Dreamhearth;

import GameplayScene;
import IScene;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class ScenePlayground : public GameplayScene
{
public:
	explicit ScenePlayground(dh::RenderContext const & render_context, SceneTransition const & transition);
};

ScenePlayground::ScenePlayground(dh::RenderContext const & render_context, SceneTransition const & transition)
	: GameplayScene(render_context, SceneId::Playground, transition)
{
}
