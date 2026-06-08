// IScene.ixx

module;

#include <optional>

export module IScene;

import Input;
import SniffTheWayConstants;

export struct SceneTransition
{
	SniffTheWay::SceneId next_scene_id;
};

export class IScene
{
public:
    virtual ~IScene() = default;

    virtual void OnWindowResized(int width, int height) {};

    // Returns a transition if the scene wants to hand off control,
    // or nullopt to keep running.
    // Returns an empty transition to signal "exit the application".
    virtual std::optional<SceneTransition> Update(float dt, Input const & input) = 0;

	// Called after the renderer has waited for the current frame's GPU work,
	// giving scenes a safe point to destroy retired GPU assets.
	virtual void DestroyPendingAssets() const {}

    virtual void Render() const = 0;
};
