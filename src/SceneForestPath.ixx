// SceneForestPath.ixx

module;

export module SceneForestPath;

import Dreamhearth;

import GameplayScene;
import IScene;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneForestPath : public GameplayScene
{
public:
	explicit SceneForestPath(dh::RenderContext const & render_context, SceneTransition const & transition);
};

SceneForestPath::SceneForestPath(dh::RenderContext const & render_context, SceneTransition const & transition)
	: GameplayScene(render_context, SceneId::ForestPath, transition)
{
}
