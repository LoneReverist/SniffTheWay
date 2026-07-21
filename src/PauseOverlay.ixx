// PauseOverlay.ixx

module;

#include <optional>
#include <string>
#include <string_view>

#include <glm/glm.hpp>

export module PauseOverlay;

import AssetManager;
import AssetPool;
import AudioSystem;
import Camera;
import ColorPipeline;
import FontAtlas;
import GameViewport;
import Input;
import SceneRenderer;
import SniffTheWayConstants;
import TextPipeline;
import UIDarkBackdrop;
import UILabel;

using namespace SniffTheWay;

export enum class PauseAction
{
	None,
	Resume,
	Settings,
	ReturnToTitle,
	Exit,
};

export class PauseOverlay
{
public:
	void Init(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		Camera2d const & camera2d,
		FontAtlas const & font_atlas,
		AudioSystem & audio_system);

	PauseAction Update(Input const & input, GameViewport const & viewport);
	void SetVisible(bool visible);
	bool IsVisible() const { return m_visible; }

private:
	enum class Button
	{
		None,
		Resume,
		Settings,
		ReturnToTitle,
		Exit,
	};

	Button hovered_button(std::optional<glm::vec2> pointer_position) const;
	PauseAction action_for_button(Button button) const;
	void update_button_colors();

private:
	SceneRenderer * m_renderer = nullptr;
	AudioSystem * m_audio_system = nullptr;
	UIDarkBackdrop m_backdrop;
	UILabel m_title_label;
	UILabel m_resume_label;
	UILabel m_settings_label;
	UILabel m_return_to_title_label;
	UILabel m_exit_label;
	AssetId m_backdrop_ro_id;
	AssetId m_title_ro_id;
	AssetId m_resume_ro_id;
	AssetId m_settings_ro_id;
	AssetId m_return_to_title_ro_id;
	AssetId m_exit_ro_id;
	Button m_selected_button = Button::Resume;
	Button m_armed_button = Button::None;
	glm::vec2 m_last_pointer_position{ 0.0f };
	bool m_has_last_pointer_position = false;
	bool m_visible = false;
};

void PauseOverlay::Init(
	AssetManager & asset_manager,
	SceneRenderer & renderer,
	Camera2d const & camera2d,
	FontAtlas const & font_atlas,
	AudioSystem & audio_system)
{
	m_renderer = &renderer;
	m_audio_system = &audio_system;

	const auto color_pipeline_id = asset_manager.AddPipeline<ColorPipeline>(camera2d);
	const auto text_pipeline_id = asset_manager.AddPipeline<TextPipeline>(camera2d, asset_manager);

	m_backdrop.Init(
		asset_manager,
		0.0f,
		UIWidth,
		0.0f,
		UIHeight,
		0.78f /*alpha_top*/,
		0.78f /*alpha_bottom*/);
	m_backdrop_ro_id = renderer.CreateRenderObject(
		"pause backdrop",
		RenderLayer::OverlayBackdrop,
		m_backdrop.GetMeshId(),
		color_pipeline_id);
	m_backdrop.SetROId(m_backdrop_ro_id);

	auto init_label = [&](UILabel & label, AssetId & ro_id, std::string_view name, std::string_view text,
		float font_size, glm::vec2 origin)
	{
		label.Init(asset_manager, text, font_atlas, font_size, origin, UILabel::Align::Center, StoryTextColor);
		ro_id = renderer.CreateRenderObject(
			std::string{ name },
			RenderLayer::OverlayForeground,
			label.GetMeshId(),
			text_pipeline_id,
			label.GetPipelineData());
		label.SetROId(ro_id);
	};

	init_label(m_title_label, m_title_ro_id, "pause title", "PAUSED",
		StoryLargeFontSize, glm::vec2{ UIWidth * 0.5f, 300.0f });
	init_label(m_resume_label, m_resume_ro_id, "pause resume", "Resume",
		StoryMediumFontSize, glm::vec2{ UIWidth * 0.5f, 460.0f });
	init_label(m_settings_label, m_settings_ro_id, "pause settings", "Settings",
		StoryMediumFontSize, glm::vec2{ UIWidth * 0.5f, 580.0f });
	init_label(m_return_to_title_label, m_return_to_title_ro_id, "pause return to title", "Return to Title",
		StoryMediumFontSize, glm::vec2{ UIWidth * 0.5f, 700.0f });
	init_label(m_exit_label, m_exit_ro_id, "pause exit", "Exit",
		StoryMediumFontSize, glm::vec2{ UIWidth * 0.5f, 820.0f });

	SetVisible(false);
}

PauseAction PauseOverlay::Update(Input const & input, GameViewport const & viewport)
{
	if (!m_visible)
		return PauseAction::None;

	const std::optional<glm::vec2> pointer_position = viewport.FramebufferToUI(input.GetMousePos());
	if (pointer_position.has_value())
	{
		const glm::vec2 position = pointer_position.value();
		const bool pointer_moved = !m_has_last_pointer_position
			|| position.x != m_last_pointer_position.x
			|| position.y != m_last_pointer_position.y;
		if (pointer_moved)
		{
			const Button hovered = hovered_button(pointer_position);
			if (hovered != Button::None)
				m_selected_button = hovered;
			m_last_pointer_position = position;
			m_has_last_pointer_position = true;
		}
	}

	if (input.KeyJustPressed(Input::Key::Up) || input.KeyJustPressed('W'))
	{
		switch (m_selected_button)
		{
		case Button::Resume: m_selected_button = Button::Exit; break;
		case Button::Settings: m_selected_button = Button::Resume; break;
		case Button::ReturnToTitle: m_selected_button = Button::Settings; break;
		case Button::Exit: m_selected_button = Button::ReturnToTitle; break;
		case Button::None: m_selected_button = Button::Resume; break;
		}
	}
	if (input.KeyJustPressed(Input::Key::Down) || input.KeyJustPressed('S'))
	{
		switch (m_selected_button)
		{
		case Button::Resume: m_selected_button = Button::Settings; break;
		case Button::Settings: m_selected_button = Button::ReturnToTitle; break;
		case Button::ReturnToTitle: m_selected_button = Button::Exit; break;
		case Button::Exit: m_selected_button = Button::Resume; break;
		case Button::None: m_selected_button = Button::Resume; break;
		}
	}

	if (input.MouseButtonJustPressed(Input::MouseButton::Left))
		m_armed_button = hovered_button(pointer_position);

	PauseAction action = PauseAction::None;
	if (input.MouseButtonJustReleased(Input::MouseButton::Left))
	{
		const Button released_over = hovered_button(pointer_position);
		if (m_armed_button != Button::None && released_over == m_armed_button)
			action = action_for_button(m_armed_button);
		m_armed_button = Button::None;
	}

	if ((input.KeyJustPressed(Input::Key::Enter) && !input.AltIsDown())
		|| input.KeyJustPressed(Input::Key::Space))
	{
		action = action_for_button(m_selected_button);
	}

	update_button_colors();
	if (action != PauseAction::None && m_audio_system)
		m_audio_system->PlaySound(SoundCue::ShortChime);
	return action;
}

void PauseOverlay::SetVisible(bool visible)
{
	m_visible = visible;
	m_selected_button = Button::Resume;
	m_armed_button = Button::None;
	m_has_last_pointer_position = false;

	if (!m_renderer)
		return;

	m_renderer->Show(m_backdrop_ro_id, visible);
	m_renderer->Show(m_title_ro_id, visible);
	m_renderer->Show(m_resume_ro_id, visible);
	m_renderer->Show(m_settings_ro_id, visible);
	m_renderer->Show(m_return_to_title_ro_id, visible);
	m_renderer->Show(m_exit_ro_id, visible);
	update_button_colors();
}

PauseOverlay::Button PauseOverlay::hovered_button(std::optional<glm::vec2> pointer_position) const
{
	if (!pointer_position.has_value())
		return Button::None;

	auto contains = [&](UILabel const & label)
	{
		const UILabel::Bounds & bounds = label.GetBounds();
		return bounds.is_valid
			&& pointer_position->x >= bounds.min.x
			&& pointer_position->x <= bounds.max.x
			&& pointer_position->y >= bounds.min.y
			&& pointer_position->y <= bounds.max.y;
	};

	if (contains(m_resume_label))
		return Button::Resume;
	if (contains(m_settings_label))
		return Button::Settings;
	if (contains(m_return_to_title_label))
		return Button::ReturnToTitle;
	if (contains(m_exit_label))
		return Button::Exit;
	return Button::None;
}

PauseAction PauseOverlay::action_for_button(Button button) const
{
	switch (button)
	{
	case Button::Resume:
		return PauseAction::Resume;
	case Button::Settings:
		return PauseAction::Settings;
	case Button::ReturnToTitle:
		return PauseAction::ReturnToTitle;
	case Button::Exit:
		return PauseAction::Exit;
	case Button::None:
		return PauseAction::None;
	}

	return PauseAction::None;
}

void PauseOverlay::update_button_colors()
{
	// tried making these colors constexpr, but msvc produces corrupted code
	const glm::vec4 SelectedColor{ 1.0f, 0.72f, 0.22f, 1.0f };
	const glm::vec4 UnselectedColor = StoryTextColor;
	const bool resume_selected = m_selected_button == Button::Resume;
	const bool settings_selected = m_selected_button == Button::Settings;
	const bool return_to_title_selected = m_selected_button == Button::ReturnToTitle;
	const bool exit_selected = m_selected_button == Button::Exit;

	m_resume_label.SetText(resume_selected ? ">  Resume  <" : "Resume");
	m_resume_label.SetTextColor(resume_selected ? SelectedColor : UnselectedColor);
	m_settings_label.SetText(settings_selected ? ">  Settings  <" : "Settings");
	m_settings_label.SetTextColor(settings_selected ? SelectedColor : UnselectedColor);
	m_return_to_title_label.SetText(
		return_to_title_selected ? ">  Return to Title  <" : "Return to Title");
	m_return_to_title_label.SetTextColor(
		return_to_title_selected ? SelectedColor : UnselectedColor);
	m_exit_label.SetText(exit_selected ? ">  Exit  <" : "Exit");
	m_exit_label.SetTextColor(exit_selected ? SelectedColor : UnselectedColor);
}
