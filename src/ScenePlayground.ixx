// ScenePlayground.ixx

module;

export module ScenePlayground;

import Dreamhearth;

import GameplayScene;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class ScenePlayground : public GameplayScene
{
public:
	explicit ScenePlayground(dh::RenderContext const & render_context);
};

ScenePlayground::ScenePlayground(dh::RenderContext const & render_context)
	: GameplayScene(render_context, SceneId::Playground)
{
}
