// GameplayMessageOverlay.ixx

module;

#include <algorithm>

#include <glm/glm.hpp>

export module GameplayMessageOverlay;

import AssetManager;
import Camera;
import FontAtlas;
import GameplayMessageData;
import SceneRenderer;
import SniffTheWayConstants;
import UIShadowedLabel;
import UILabel;

using namespace SniffTheWay;

export class GameplayMessageOverlay
{
public:
	void Init(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		Camera2d const & camera2d,
		FontAtlas const & font_atlas);

	void Show(GameplayMessage const & message);
	void Hide();
	void Update(float dt);
	void RenderOffscreenTexture() const;

	bool IsVisible() const { return m_state != State::Hidden; }

private:
	enum class State
	{
		Hidden,
		FadingIn,
		Holding,
		FadingOut,
	};

	void set_state_and_opacity(State state, float opacity);

private:
	UIShadowedLabel m_label;
	GameplayMessage m_message;
	State m_state = State::Hidden;
	float m_elapsed = 0.0f;
};

void GameplayMessageOverlay::Init(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	Camera2d const & camera2d,
	FontAtlas const & font_atlas)
{
	m_label.Init(
		asset_manager,
		renderer,
		camera2d,
		"gameplay message",
		"",
		font_atlas,
		StoryMediumFontSize,
		glm::vec2{ UIWidth * 0.5f, UIHeight * 0.85f },
		UILabel::Align::Center,
		StoryTextColor);
	m_label.SetVisible(false);
}

void GameplayMessageOverlay::Show(GameplayMessage const & message)
{
	if (message.text.empty())
	{
		Hide();
		return;
	}

	m_message = message;
	m_message.hold_duration = std::max(m_message.hold_duration, 0.0f);
	m_message.fade_in_duration = std::max(m_message.fade_in_duration, 0.0f);
	m_message.fade_out_duration = std::max(m_message.fade_out_duration, 0.0f);
	m_elapsed = 0.0f;

	m_label.SetText(m_message.text);
	m_label.SetVisible(true);
	if (m_message.fade_in_duration > 0.0f)
		set_state_and_opacity(State::FadingIn, 0.0f);
	else
		set_state_and_opacity(State::Holding, 1.0f);
}

void GameplayMessageOverlay::Hide()
{
	m_state = State::Hidden;
	m_elapsed = 0.0f;
	m_label.SetVisible(false);
}

void GameplayMessageOverlay::Update(float dt)
{
	if (m_state == State::Hidden)
		return;

	m_elapsed += std::max(dt, 0.0f);

	const float fade_in_end = m_message.fade_in_duration;
	const float hold_end = fade_in_end + m_message.hold_duration;
	const float fade_out_end = hold_end + m_message.fade_out_duration;

	if (m_elapsed < fade_in_end)
	{
		set_state_and_opacity(State::FadingIn, m_elapsed / m_message.fade_in_duration);
		return;
	}

	if (m_elapsed < hold_end)
	{
		set_state_and_opacity(State::Holding, 1.0f);
		return;
	}

	if (m_message.fade_out_duration > 0.0f && m_elapsed < fade_out_end)
	{
		set_state_and_opacity(State::FadingOut, 1.0f - (m_elapsed - hold_end) / m_message.fade_out_duration);
		return;
	}

	Hide();
}

void GameplayMessageOverlay::RenderOffscreenTexture() const
{
	if (IsVisible())
		m_label.RenderOffscreenTexture();
}

void GameplayMessageOverlay::set_state_and_opacity(State state, float opacity)
{
	m_state = state;
	m_label.SetOpacity(opacity);
}
