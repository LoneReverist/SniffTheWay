// SceneManager.ixx

module;

#include <algorithm>
#include <memory>
#include <optional>

export module SceneManager;

import Dreamhearth;

import AudioSystem;
import GameViewport;
import Input;
import IScene;
import Playthrough;
import SceneRegistry;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneManager
{
public:
    SceneManager(AudioSystem & audio_system, dh::RenderContext const & ctx, SceneTransition initial_trans);

    void OnWindowResized(int w, int h);

    // An external transition takes priority over scene input while active.
    void Update(float dt, Input const & input, std::optional<SceneTransition> transition = std::nullopt);
    SceneId GetCurrentSceneId() const { return m_cur_scene_id; }
    void Render() const;

    // Destroys the old scene (and its GPU resources) safely, then builds the next.
    // Returns false when the app should exit.
    bool ApplyPendingTransition(Playthrough * playthrough);

    bool HasPendingTransition() const;
    SceneTransition const * GetPendingTransition() const;

private:
	enum class TransitionState
	{
		FadingIn,
		Active,
		FadingOut,
		ReadyToSwitch,
	};

	void set_transition_opacity(float opacity);
	static bool is_story_gameplay_transition(SceneTransition const & transition);

	static constexpr float DefaultTransitionDuration = 0.5f;
	static constexpr float StoryGameplayTransitionDuration = 1.5f;

	AudioSystem & m_audio_system;
    dh::RenderContext const & m_render_context;
	GameViewport m_game_viewport;

	SceneRegistry m_scene_registry;
    std::unique_ptr<IScene> m_cur_scene;
	SceneId m_cur_scene_id;
    std::optional<SceneTransition> m_pending_scene_transition;
	TransitionState m_transition_state = TransitionState::FadingIn;
	float m_transition_opacity = 1.0f;
	float m_transition_duration = DefaultTransitionDuration;
	bool m_fade_audio = false;
};

SceneManager::SceneManager(AudioSystem & audio_system, dh::RenderContext const & ctx, SceneTransition initial_trans)
	: m_audio_system{ audio_system }
	, m_render_context{ctx}
	, m_cur_scene_id{ initial_trans.next_scene_id }
{
	m_cur_scene = m_scene_registry.Create(initial_trans, m_render_context, m_audio_system, nullptr);
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

void SceneManager::Update(float dt, Input const & input, std::optional<SceneTransition> transition)
{
	if (!m_cur_scene)
		return;

	const float transition_step = std::max(dt, 0.0f) / m_transition_duration;
	switch (m_transition_state)
	{
	case TransitionState::FadingIn:
		set_transition_opacity(m_transition_opacity - transition_step);
		if (m_transition_opacity <= 0.0f)
			m_transition_state = TransitionState::Active;
		break;
	case TransitionState::Active:
	{
		if (!transition)
			transition = m_cur_scene->Update(dt, input);
		if (transition)
		{
			m_pending_scene_transition = std::move(transition);
			m_fade_audio = is_story_gameplay_transition(*m_pending_scene_transition);
			m_transition_duration = m_fade_audio
				? StoryGameplayTransitionDuration
				: DefaultTransitionDuration;
			m_transition_state = TransitionState::FadingOut;
		}
		break;
	}
	case TransitionState::FadingOut:
		set_transition_opacity(m_transition_opacity + transition_step);
		if (m_fade_audio)
			m_audio_system.SetTransitionVolume(1.0f - m_transition_opacity);
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

bool SceneManager::ApplyPendingTransition(Playthrough * playthrough)
{
	if (!HasPendingTransition())
		return true;

	SceneTransition const transition = *m_pending_scene_transition;
	m_audio_system.SetTransitionVolume(1.0f);
	m_render_context.WaitForLastFrame(); // GPU drains before old scene dies
	m_cur_scene.reset(); // GPU resources destroyed here — safe because we waited

	m_cur_scene = m_scene_registry.Create(transition, m_render_context, m_audio_system, playthrough);
	m_cur_scene_id = transition.next_scene_id;
	if (!m_cur_scene)
		return false;

	m_cur_scene->OnViewportChanged(m_game_viewport);
	m_transition_state = TransitionState::FadingIn;
	set_transition_opacity(1.0f);

	m_pending_scene_transition = std::nullopt;
	return true;
}

bool SceneManager::is_story_gameplay_transition(SceneTransition const & transition)
{
	if (!transition.previous_scene_id)
		return false;

	const bool leaving_story = IsStoryScene(*transition.previous_scene_id);
	const bool entering_story = IsStoryScene(transition.next_scene_id);
	const bool leaving_gameplay = IsGameplayScene(*transition.previous_scene_id);
	const bool entering_gameplay = IsGameplayScene(transition.next_scene_id);
	return (leaving_story && entering_gameplay) || (leaving_gameplay && entering_story);
}

bool SceneManager::HasPendingTransition() const
{
	return m_transition_state == TransitionState::ReadyToSwitch
		&& m_pending_scene_transition.has_value();
}

SceneTransition const * SceneManager::GetPendingTransition() const
{
	return HasPendingTransition() ? &*m_pending_scene_transition : nullptr;
}

void SceneManager::set_transition_opacity(float opacity)
{
	m_transition_opacity = std::clamp(opacity, 0.0f, 1.0f);
	if (m_cur_scene)
		m_cur_scene->SetTransitionOpacity(m_transition_opacity);
}
