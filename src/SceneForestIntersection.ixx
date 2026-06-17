// SceneForestIntersection.ixx

module;

export module SceneForestIntersection;

import Dreamhearth;

import GameplayScene;
import IScene;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneForestIntersection : public GameplayScene
{
public:
	explicit SceneForestIntersection(dh::RenderContext const & render_context, SceneTransition const & transition);
};

SceneForestIntersection::SceneForestIntersection(dh::RenderContext const & render_context, SceneTransition const & transition)
	: GameplayScene(render_context, SceneId::ForestIntersection, transition)
{
}
