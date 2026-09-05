// Game.ixx

module;

#include <optional>
#include <utility>

export module Game;

import Dreamhearth;

import AudioSystem;
import Input;
import IScene;
import Playthrough;
import SceneManager;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class Game
{
public:
	explicit Game(dh::RenderContext render_context);

	Game(Game const &) = delete;
	Game & operator=(Game const &) = delete;
	Game(Game &&) = delete;
	Game & operator=(Game &&) = delete;

	dh::RenderContext & GetRenderContext() { return m_render_context; }

	void OnWindowResized(int width, int height);
	void Update(float dt, Input const & input);
	void Render() const;

	// Applies a pending scene transition once its fade-out has completed.
	// Returns false when the application should exit.
	bool ApplyPendingTransition();

private:
	void update_audio(SceneId scene_id);
#ifdef _DEBUG
	std::optional<SceneTransition> get_debug_transition(Input const & input) const;
#endif

private:
	// SceneManager holds references to these objects, so their declaration order
	// and Game's stable address are part of the ownership contract.
	dh::RenderContext m_render_context;
	AudioSystem m_audio_system;
	std::optional<Playthrough> m_playthrough;
	SceneManager m_scene_manager;
};

Game::Game(dh::RenderContext render_context)
	: m_render_context{ std::move(render_context) }
	, m_audio_system{}
	, m_playthrough{}
	, m_scene_manager{ m_audio_system, m_render_context, SceneTransition{ SceneId::Title } }
{
	update_audio(SceneId::Title);
}

void Game::OnWindowResized(int width, int height)
{
	m_scene_manager.OnWindowResized(width, height);
}

void Game::Update(float dt, Input const & input)
{
#ifdef _DEBUG
	m_scene_manager.Update(dt, input, get_debug_transition(input));
#else
	m_scene_manager.Update(dt, input);
#endif
}

#ifdef _DEBUG
std::optional<SceneTransition> Game::get_debug_transition(Input const & input) const
{
	if (!input.ControlIsDown())
		return std::nullopt;

	const SceneId current_scene_id = m_scene_manager.GetCurrentSceneId();
	if (!IsStoryScene(current_scene_id) && !IsGameplayScene(current_scene_id))
		return std::nullopt;

	constexpr SceneId story_scenes[] = { SceneId::Picnic, SceneId::Creek, SceneId::Night, SceneId::Home };
	for (int i = 0; i < 4; ++i)
	{
		if (input.KeyJustPressed('1' + i))
			return SceneTransition{ story_scenes[i], current_scene_id };
	}
	return std::nullopt;
}
#endif

void Game::Render() const
{
	m_scene_manager.Render();
}

bool Game::ApplyPendingTransition()
{
	bool end_playthrough = false;
	if (SceneTransition const * transition = m_scene_manager.GetPendingTransition())
	{
		switch (transition->playthrough_action)
		{
		case PlaythroughAction::StartNew:
			m_playthrough.emplace();
			break;
		case PlaythroughAction::End:
			end_playthrough = true;
			break;
		case PlaythroughAction::None:
			break;
		}

		update_audio(transition->next_scene_id);
	}

	const bool should_continue = m_scene_manager.ApplyPendingTransition(
		m_playthrough ? &*m_playthrough : nullptr);
		
	if (end_playthrough)
		m_playthrough.reset();

	return should_continue;
}

void Game::update_audio(SceneId scene_id)
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
