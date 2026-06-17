// ScenePlayground2.ixx

module;

export module ScenePlayground2;

import Dreamhearth;

import GameplayScene;
import IScene;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class ScenePlayground2 : public GameplayScene
{
public:
	explicit ScenePlayground2(dh::RenderContext const & render_context, SceneTransition const & transition);
};

ScenePlayground2::ScenePlayground2(dh::RenderContext const & render_context, SceneTransition const & transition)
	: GameplayScene(render_context, SceneId::Playground2, transition)
{
}
