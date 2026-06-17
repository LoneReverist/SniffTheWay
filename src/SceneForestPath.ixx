// SceneForestPath.ixx

module;

export module SceneForestPath;

import Dreamhearth;

import GameplayScene;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneForestPath : public GameplayScene
{
public:
	explicit SceneForestPath(dh::RenderContext const & render_context);
};

SceneForestPath::SceneForestPath(dh::RenderContext const & render_context)
	: GameplayScene(render_context, SceneId::ForestPath)
{
}
