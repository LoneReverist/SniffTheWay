// SceneRegistry.ixx

module;

#include <memory>

export module SceneRegistry;

import Dreamhearth;

import GameplayScene;
import IScene;
import SniffTheWayConstants;
import StoryScene;
import TitleScene;

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
	if (trans.next_scene_id == SceneId::Title)
		return std::make_unique<TitleScene>(ctx);

	if (IsStoryScene(trans.next_scene_id))
		return std::make_unique<StoryScene>(ctx, trans.next_scene_id);

	if (IsGameplayScene(trans.next_scene_id))
		return std::make_unique<GameplayScene>(ctx, trans.next_scene_id, trans);

	return nullptr;
}
