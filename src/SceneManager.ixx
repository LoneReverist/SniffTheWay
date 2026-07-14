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
import SceneRegistry;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneManager
{
public:
    SceneManager(AudioSystem & audio_system, dh::RenderContext const & ctx, SceneTransition initial_trans);

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
	void update_audio(SniffTheWay::SceneId scene_id);

	static constexpr float TransitionDuration = 0.5f;

	AudioSystem & m_audio_system;
    dh::RenderContext const & m_render_context;
	GameViewport m_game_viewport;

	SceneRegistry m_scene_registry;
    std::unique_ptr<IScene> m_cur_scene;
    std::optional<SceneTransition> m_pending_scene_transition;
	TransitionState m_transition_state = TransitionState::FadingIn;
	float m_transition_opacity = 1.0f;
};

SceneManager::SceneManager(AudioSystem & audio_system, dh::RenderContext const & ctx, SceneTransition initial_trans)
	: m_audio_system{ audio_system }
	, m_render_context{ctx}
{
	update_audio(initial_trans.next_scene_id);
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
	update_audio(transition.next_scene_id);
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

void SceneManager::update_audio(SceneId scene_id)
{
	switch (scene_id)
	{
	case SceneId::Title:
		m_audio_system.PlayMusic(MusicCue::Title);
		m_audio_system.StopAmbience();
		break;
	case SceneId::Picnic:
		m_audio_system.PlayMusic(MusicCue::Picnic);
		m_audio_system.StopAmbience();
		break;
	case SceneId::ForestPath:
	case SceneId::ForestPath2:
	case SceneId::RightTurn:
	case SceneId::ForestLake:
	case SceneId::Playground:
	case SceneId::ForestHorizontal:
	case SceneId::ThickerForestTransition:
	case SceneId::BeforeCreek:
		m_audio_system.PlayMusic(MusicCue::EarlyForest);
		m_audio_system.PlayAmbience(AmbienceCue::EarlyForest);
		break;
	case SceneId::Creek:
		m_audio_system.PlayMusic(MusicCue::Creek);
		m_audio_system.PlayAmbience(AmbienceCue::Creek);
		break;
	case SceneId::AfterCreek:
	case SceneId::GoldenIntersection:
	case SceneId::FallenTree:
	case SceneId::GoldenPath:
	case SceneId::GoldenHour:
	case SceneId::DeepForest:
	case SceneId::DarkForest:
	case SceneId::DarkForest2:
		m_audio_system.PlayMusic(MusicCue::MiddleForest);
		m_audio_system.PlayAmbience(AmbienceCue::MiddleForest);
		break;
	case SceneId::Night:
		m_audio_system.PlayMusic(MusicCue::Night);
		m_audio_system.StopAmbience();
		break;
	case SceneId::MorningForest:
	case SceneId::MorningForest2:
	case SceneId::DirtPath:
	case SceneId::DirtIntersection:
	case SceneId::BenchPath:
	case SceneId::HousePath:
		m_audio_system.PlayMusic(MusicCue::LateForest);
		m_audio_system.PlayAmbience(AmbienceCue::LateForest);
		break;
	case SceneId::Home:
	case SceneId::Credits:
		m_audio_system.PlayMusic(MusicCue::Home);
		m_audio_system.StopAmbience();
		break;
	default:
		m_audio_system.StopMusic();
		m_audio_system.StopAmbience();
		break;
	}
}
