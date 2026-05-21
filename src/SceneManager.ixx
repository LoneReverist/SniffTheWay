// SceneManager.ixx

module;

#include <memory>
#include <optional>

export module SceneManager;

import Dreamhearth;
namespace dh = Dreamhearth;

import Input;
import IScene;

export class SceneManager
{
public:
    SceneManager(dh::RenderContext const & ctx, std::unique_ptr<IScene> initial_scene);

    void OnViewportResized(int w, int h);
    void OnDPIScaleFactorChanged(float dpi_scale);

    // Returns false when the app should exit.
    bool Update(float dt, Input const & input);
    void Render() const;

    // Call this ONLY after render_context.WaitForLastFrame().
    // Destroys the old scene (and its GPU resources) safely, then builds the next.
    void ApplyPendingTransition();

    bool HasPendingTransition() const { return m_pending_scene_transition.has_value(); }

private:
    dh::RenderContext const & m_render_context;
    float m_dpi_scale = 0.0f;
    int m_viewport_w = 0, m_viewport_h = 0;

    std::unique_ptr<IScene> m_cur_scene;
    std::optional<SceneTransition> m_pending_scene_transition;
};

SceneManager::SceneManager(dh::RenderContext const & ctx, std::unique_ptr<IScene> initial_scene)
	: m_render_context{ctx}
	, m_cur_scene(std::move(initial_scene))
{
}

void SceneManager::OnViewportResized(int w, int h)
{
	m_viewport_w = w;
	m_viewport_h = h;
	if (m_cur_scene)
		m_cur_scene->OnViewportResized(w, h);
}

void SceneManager::OnDPIScaleFactorChanged(float dpi_scale)
{
	m_dpi_scale = dpi_scale;
	if (m_cur_scene)
		m_cur_scene->OnDPIScaleFactorChanged(dpi_scale);
}

// Returns false when the app should exit.
bool SceneManager::Update(float dt, Input const & input)
{
	if (!m_cur_scene)
		return false;

	m_pending_scene_transition = m_cur_scene->Update(dt, input);

	if (m_pending_scene_transition.has_value() && !m_pending_scene_transition.value().create_scene_fn)
		return false; // scene signalled exit

	return true;
}

void SceneManager::Render() const
{
	if (m_cur_scene)
		m_cur_scene->Render();
}

void SceneManager::ApplyPendingTransition()
{
	if (!HasPendingTransition())
		return;

	m_render_context.WaitForLastFrame(); // GPU drains before old scene dies
	m_cur_scene.reset(); // GPU resources destroyed here — safe because we waited

	m_cur_scene = m_pending_scene_transition.value().create_scene_fn(m_render_context);
	m_cur_scene->OnDPIScaleFactorChanged(m_dpi_scale);
	m_cur_scene->OnViewportResized(m_viewport_w, m_viewport_h);

	m_pending_scene_transition = std::nullopt;
}
