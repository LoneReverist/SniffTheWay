// SceneManager.ixx

module;

#include <algorithm>
#include <memory>
#include <optional>

export module SceneManager;

import Dreamhearth;

import GameViewport;
import Input;
import IScene;
import SceneRegistry;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

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

    bool HasPendingTransition() const;

private:
	enum class TransitionState
	{
		FadingIn,
		Active,
		FadingOut,
		ReadyToSwitch,
	};

	void set_transition_opacity(float opacity);

	static constexpr float TransitionDuration = 0.5f;

    dh::RenderContext const & m_render_context;
	GameViewport m_game_viewport;

	SceneRegistry m_scene_registry;
    std::unique_ptr<IScene> m_cur_scene;
    std::optional<SceneTransition> m_pending_scene_transition;
	TransitionState m_transition_state = TransitionState::FadingIn;
	float m_transition_opacity = 1.0f;
};

SceneManager::SceneManager(dh::RenderContext const & ctx, SceneTransition initial_trans)
	: m_render_context{ctx}
{
	m_cur_scene = m_scene_registry.Create(initial_trans, m_render_context);
	if (m_cur_scene)
		m_cur_scene->SetTransitionOpacity(m_transition_opacity);
}

void SceneManager::OnWindowResized(int w, int h)
{
	m_render_context.WaitForLastFrame(); // GPU drains before recreating meshes

	m_game_viewport = CalculateGameViewport(w, h);
	if (m_cur_scene)
		m_cur_scene->OnViewportChanged(m_game_viewport);
}

// Returns false when the app should exit.
void SceneManager::Update(float dt, Input const & input)
{
	if (!m_cur_scene)
		return;

	const float transition_step = std::max(dt, 0.0f) / TransitionDuration;
	switch (m_transition_state)
	{
	case TransitionState::FadingIn:
		set_transition_opacity(m_transition_opacity - transition_step);
		if (m_transition_opacity <= 0.0f)
			m_transition_state = TransitionState::Active;
		break;
	case TransitionState::Active:
		if (std::optional<SceneTransition> transition = m_cur_scene->Update(dt, input))
		{
			m_pending_scene_transition = std::move(transition);
			m_transition_state = TransitionState::FadingOut;
		}
		break;
	case TransitionState::FadingOut:
		set_transition_opacity(m_transition_opacity + transition_step);
		if (m_transition_opacity >= 1.0f)
			m_transition_state = TransitionState::ReadyToSwitch;
		break;
	case TransitionState::ReadyToSwitch:
		break;
	}
}

void SceneManager::Render() const
{
	if (m_cur_scene)
	{
		m_cur_scene->DestroyPendingAssets();
		m_cur_scene->Render();
	}
}

bool SceneManager::ApplyPendingTransition()
{
	if (!HasPendingTransition())
		return true;

	SceneTransition const transition = *m_pending_scene_transition;
	m_render_context.WaitForLastFrame(); // GPU drains before old scene dies
	m_cur_scene.reset(); // GPU resources destroyed here — safe because we waited

	m_cur_scene = m_scene_registry.Create(transition, m_render_context);
	if (!m_cur_scene)
		return false;

	m_cur_scene->OnViewportChanged(m_game_viewport);
	m_transition_state = TransitionState::FadingIn;
	set_transition_opacity(1.0f);

	m_pending_scene_transition = std::nullopt;
	return true;
}

bool SceneManager::HasPendingTransition() const
{
	return m_transition_state == TransitionState::ReadyToSwitch
		&& m_pending_scene_transition.has_value();
}

void SceneManager::set_transition_opacity(float opacity)
{
	m_transition_opacity = std::clamp(opacity, 0.0f, 1.0f);
	if (m_cur_scene)
		m_cur_scene->SetTransitionOpacity(m_transition_opacity);
}
