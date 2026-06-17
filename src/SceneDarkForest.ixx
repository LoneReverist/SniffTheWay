// SceneDarkForest.ixx

module;

export module SceneDarkForest;

import Dreamhearth;

import GameplayScene;
import IScene;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneDarkForest : public GameplayScene
{
public:
	explicit SceneDarkForest(dh::RenderContext const & render_context, SceneTransition const & transition);
};

SceneDarkForest::SceneDarkForest(dh::RenderContext const & render_context, SceneTransition const & transition)
	: GameplayScene(render_context, SceneId::DarkForest, transition)
{
}
