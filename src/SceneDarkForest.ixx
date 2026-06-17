// SceneDarkForest.ixx

module;

export module SceneDarkForest;

import Dreamhearth;

import GameplayScene;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneDarkForest : public GameplayScene
{
public:
	explicit SceneDarkForest(dh::RenderContext const & render_context);
};

SceneDarkForest::SceneDarkForest(dh::RenderContext const & render_context)
	: GameplayScene(render_context, SceneId::DarkForest)
{
}
