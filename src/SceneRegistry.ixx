// SceneRegistry.ixx

module;

#include <memory>

export module SceneRegistry;

import Dreamhearth;

import GameplayScene;
import IScene;
import SniffTheWayConstants;
import StoryScene;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneRegistry
{
public:
	SceneRegistry() = default;

    std::unique_ptr<IScene> Create(SceneTransition trans, dh::RenderContext const & ctx) const;
};

std::unique_ptr<IScene> SceneRegistry::Create(SceneTransition trans, dh::RenderContext const & ctx) const
{
	switch (trans.next_scene_id)
	{
	case SceneId::Picnic:
	case SceneId::Creek:
	case SceneId::Night:
	case SceneId::Home:
		return std::make_unique<StoryScene>(ctx, trans.next_scene_id);

	case SceneId::ForestPath:
	case SceneId::Playground:
	case SceneId::Playground2:
	case SceneId::DarkForest:
	case SceneId::ForestIntersection:
		return std::make_unique<GameplayScene>(ctx, trans.next_scene_id, trans);

	case SceneId::Exit:
		return nullptr;
	}

	return nullptr;
}
