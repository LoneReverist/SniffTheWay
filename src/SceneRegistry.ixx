// SceneRegistry.ixx

module;

#include <functional>
#include <memory>
#include <unordered_map>

export module SceneRegistry;

import Dreamhearth;
namespace dh = Dreamhearth;

import IScene;
import SceneForestIntersection;
import SceneForestPath;
import ScenePicnic;
import SniffTheWayConstants;

using namespace SniffTheWay;

export class SceneRegistry
{
public:
    using CreateSceneFn = std::function<std::unique_ptr<IScene>(dh::RenderContext const &)>;

    SceneRegistry();

    std::unique_ptr<IScene> Create(SceneTransition trans, dh::RenderContext const & ctx) const;

private:
    std::unordered_map<SceneId, CreateSceneFn> m_create_scene_fns;
};

SceneRegistry::SceneRegistry()
{
	m_create_scene_fns[SceneId::Picnic] =
		[](dh::RenderContext const & ctx) { return std::make_unique<ScenePicnic>(ctx); };
	m_create_scene_fns[SceneId::ForestPath] =
		[](dh::RenderContext const & ctx) { return std::make_unique<SceneForestPath>(ctx); };
	m_create_scene_fns[SceneId::ForestIntersection] =
		[](dh::RenderContext const & ctx) { return std::make_unique<SceneForestIntersection>(ctx); };
}

std::unique_ptr<IScene> SceneRegistry::Create(SceneTransition trans, dh::RenderContext const & ctx) const
{
	auto iter = m_create_scene_fns.find(trans.next_scene_id);
	if (iter == m_create_scene_fns.end())
		return nullptr;

	CreateSceneFn const & create_scene_fn = iter->second;
	return create_scene_fn(ctx);
}
