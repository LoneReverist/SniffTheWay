// IScene.ixx

module;

#include <functional>
#include <memory>
#include <optional>

export module IScene;

import Dreamhearth;
namespace dh = Dreamhearth;

import Input;

struct SceneTransition;

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

export struct SceneTransition
{
    // A factory that constructs the next scene.
    // Storing a factory (not the scene itself) means the old scene's GPU
    // resources are still alive when this struct is created — safe.
    using CreateSceneFn = std::function<std::unique_ptr<IScene>(dh::RenderContext const &)>;

    CreateSceneFn create_scene_fn; // null == exit application
};
