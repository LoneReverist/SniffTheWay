// ScenePlayground2.ixx

module;

export module ScenePlayground2;

import Dreamhearth;

import GameplayScene;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class ScenePlayground2 : public GameplayScene
{
public:
	explicit ScenePlayground2(dh::RenderContext const & render_context);
};

ScenePlayground2::ScenePlayground2(dh::RenderContext const & render_context)
	: GameplayScene(render_context, SceneId::Playground2)
{
}
