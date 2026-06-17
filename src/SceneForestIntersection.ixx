// SceneForestIntersection.ixx

module;

export module SceneForestIntersection;

import Dreamhearth;

import GameplayScene;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneForestIntersection : public GameplayScene
{
public:
	explicit SceneForestIntersection(dh::RenderContext const & render_context);
};

SceneForestIntersection::SceneForestIntersection(dh::RenderContext const & render_context)
	: GameplayScene(render_context, SceneId::ForestIntersection)
{
}
