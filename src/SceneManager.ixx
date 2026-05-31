// SceneManager.ixx

module;

#include <memory>
#include <optional>

export module SceneManager;

import Dreamhearth;
namespace dh = Dreamhearth;

import Input;
import IScene;
import SceneRegistry;

export class SceneManager
{
public:
    SceneManager(dh::RenderContext const & ctx, SceneTransition initial_trans);

    void OnWindowResized(int w, int h);

    void Update(float dt, Input const & input);
    void Render() const;

    // Destroys the old scene (and its GPU resources) safely, then builds the next.
    // Returns false when the app should exit.
    bool ApplyPendingTransition();

    bool HasPendingTransition() const { return m_pending_scene_transition.has_value(); }

private:
    dh::RenderContext const & m_render_context;
    int m_viewport_w = 0, m_viewport_h = 0;

	SceneRegistry m_scene_registry;
    std::unique_ptr<IScene> m_cur_scene;
    std::optional<SceneTransition> m_pending_scene_transition;
};

SceneManager::SceneManager(dh::RenderContext const & ctx, SceneTransition initial_trans)
	: m_render_context{ctx}
{
	m_cur_scene = m_scene_registry.Create(initial_trans, m_render_context);
}

void SceneManager::OnWindowResized(int w, int h)
{
	m_render_context.WaitForLastFrame(); // GPU drains before recreating meshes
	
	m_viewport_w = w;
	m_viewport_h = h;
	if (m_cur_scene)
		m_cur_scene->OnWindowResized(w, h);
}

// Returns false when the app should exit.
void SceneManager::Update(float dt, Input const & input)
{
	if (m_cur_scene)
		m_pending_scene_transition = m_cur_scene->Update(dt, input);
}

void SceneManager::Render() const
{
	if (m_cur_scene)
		m_cur_scene->Render();
}

bool SceneManager::ApplyPendingTransition()
{
	if (!HasPendingTransition())
		return true;

	m_render_context.WaitForLastFrame(); // GPU drains before old scene dies
	m_cur_scene.reset(); // GPU resources destroyed here — safe because we waited

	m_cur_scene = m_scene_registry.Create(m_pending_scene_transition.value(), m_render_context);
	if (!m_cur_scene)
		return false;

	m_cur_scene->OnWindowResized(m_viewport_w, m_viewport_h);

	m_pending_scene_transition = std::nullopt;
	return true;
}
