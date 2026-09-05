// SceneRegistry.ixx

module;

#include <memory>

#include <glog/logging.h>

export module SceneRegistry;

import Dreamhearth;

import AudioSystem;
import CreditsScene;
import GameplayScene;
import IScene;
import Playthrough;
import SniffTheWayConstants;
import StoryScene;
import TitleScene;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneRegistry
{
public:
	SceneRegistry() = default;

	std::unique_ptr<IScene> Create(
		SceneTransition trans,
		dh::RenderContext const & ctx,
		AudioSystem & audio_system,
		Playthrough * playthrough) const;
};

std::unique_ptr<IScene> SceneRegistry::Create(
	SceneTransition trans,
	dh::RenderContext const & ctx,
	AudioSystem & audio_system,
	Playthrough * playthrough) const
{
	if (trans.next_scene_id == SceneId::Title)
		return std::make_unique<TitleScene>(ctx, audio_system);

	if (trans.next_scene_id == SceneId::Credits)
		return std::make_unique<CreditsScene>(ctx, audio_system);

	if (IsStoryScene(trans.next_scene_id))
		return std::make_unique<StoryScene>(ctx, audio_system, trans.next_scene_id);

	if (IsGameplayScene(trans.next_scene_id))
	{
		if (!playthrough)
		{
			LOG(ERROR) << "SceneRegistry: Cannot create gameplay scene without an active playthrough.";
			return nullptr;
		}
		return std::make_unique<GameplayScene>(ctx, audio_system, *playthrough, trans.next_scene_id, trans);
	}

	return nullptr;
}
