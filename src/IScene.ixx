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

    virtual void OnViewportResized(int width, int height) = 0;
    virtual void OnDPIScaleFactorChanged(float dpi_scale_factor) = 0;

    // Returns a transition if the scene wants to hand off control,
    // or nullopt to keep running.
    // Returns an empty transition to signal "exit the application".
    virtual std::optional<SceneTransition> Update(float dt, Input const & input) = 0;

    virtual void Render() const = 0;
};
