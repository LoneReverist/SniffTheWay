// SceneRegistry.ixx

module;

#include <functional>
#include <memory>
#include <unordered_map>

export module SceneRegistry;

import Dreamhearth;

import IScene;
import SceneCreek;
import SceneDarkForest;
import SceneForestIntersection;
import SceneForestPath;
import SceneHome;
import SceneNight;
import ScenePicnic;
import ScenePlayground;
import ScenePlayground2;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneRegistry
{
public:
	using CreateSceneFn = std::function<std::unique_ptr<IScene>(SceneTransition const &, dh::RenderContext const &)>;

    SceneRegistry();

    std::unique_ptr<IScene> Create(SceneTransition trans, dh::RenderContext const & ctx) const;

private:
	std::unordered_map<SceneId, CreateSceneFn> m_create_scene_fns;
};

SceneRegistry::SceneRegistry()
{
	m_create_scene_fns[SceneId::Picnic] =
		[](SceneTransition const &, dh::RenderContext const & ctx) { return std::make_unique<ScenePicnic>(ctx); };
	m_create_scene_fns[SceneId::ForestPath] =
		[](SceneTransition const & trans, dh::RenderContext const & ctx) { return std::make_unique<SceneForestPath>(ctx, trans); };
	m_create_scene_fns[SceneId::Playground] =
		[](SceneTransition const & trans, dh::RenderContext const & ctx) { return std::make_unique<ScenePlayground>(ctx, trans); };
	m_create_scene_fns[SceneId::Playground2] =
		[](SceneTransition const & trans, dh::RenderContext const & ctx) { return std::make_unique<ScenePlayground2>(ctx, trans); };
	m_create_scene_fns[SceneId::Creek] =
		[](SceneTransition const &, dh::RenderContext const & ctx) { return std::make_unique<SceneCreek>(ctx); };
	m_create_scene_fns[SceneId::DarkForest] =
		[](SceneTransition const & trans, dh::RenderContext const & ctx) { return std::make_unique<SceneDarkForest>(ctx, trans); };
	m_create_scene_fns[SceneId::Night] =
		[](SceneTransition const &, dh::RenderContext const & ctx) { return std::make_unique<SceneNight>(ctx); };
	m_create_scene_fns[SceneId::ForestIntersection] =
		[](SceneTransition const & trans, dh::RenderContext const & ctx) { return std::make_unique<SceneForestIntersection>(ctx, trans); };
	m_create_scene_fns[SceneId::Home] =
		[](SceneTransition const &, dh::RenderContext const & ctx) { return std::make_unique<SceneHome>(ctx); };
}

std::unique_ptr<IScene> SceneRegistry::Create(SceneTransition trans, dh::RenderContext const & ctx) const
{
	auto iter = m_create_scene_fns.find(trans.next_scene_id);
	if (iter == m_create_scene_fns.end())
		return nullptr;

	CreateSceneFn const & create_scene_fn = iter->second;
	return create_scene_fn(trans, ctx);
}
