module;

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>

#include <glm/glm.hpp>

export module SettingsOverlay;

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

export class SettingsOverlay
{
public:
	void Init(
		AssetManager & asset_manager,
		SceneRenderer & renderer,
		Camera2d const & camera2d,
		FontAtlas const & font_atlas,
		AudioSystem & audio_system);

	// Returns true when the overlay should be closed.
	bool Update(Input const & input, GameViewport const & viewport);
	void SetVisible(bool visible);
	bool IsVisible() const { return m_visible; }

private:
	static constexpr float VolumeStep = 0.1f;

	enum class Row
	{
		None,
		Master,
		Music,
		SoundEffects,
		Back,
	};

	Row hovered_row(std::optional<glm::vec2> pointer_position) const;
	std::optional<float> clicked_adjustment(Row row, glm::vec2 pointer_position) const;
	void adjust_selected(float amount);
	void update_labels();
	static std::string volume_text(std::string_view name, float volume, bool selected);

private:
	SceneRenderer * m_renderer = nullptr;
	AudioSystem * m_audio_system = nullptr;
	UIDarkBackdrop m_backdrop;
	UILabel m_title_label;
	UILabel m_master_label;
	UILabel m_music_label;
	UILabel m_sound_effects_label;
	UILabel m_back_label;
	UILabel m_help_label;
	AssetId m_backdrop_ro_id;
	AssetId m_title_ro_id;
	AssetId m_master_ro_id;
	AssetId m_music_ro_id;
	AssetId m_sound_effects_ro_id;
	AssetId m_back_ro_id;
	AssetId m_help_ro_id;
	Row m_selected_row = Row::Master;
	std::optional<Row> m_armed_row;
	glm::vec2 m_last_pointer_position{ 0.0f };
	bool m_has_last_pointer_position = false;
	bool m_visible = false;
};

void SettingsOverlay::Init(
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

	m_backdrop.Init(asset_manager, 0.0f, UIWidth, 0.0f, UIHeight, 0.82f, 0.82f);
	m_backdrop_ro_id = renderer.CreateRenderObject(
		"settings backdrop", RenderLayer::OverlayBackdrop, m_backdrop.GetMeshId(), color_pipeline_id);
	m_backdrop.SetROId(m_backdrop_ro_id);

	auto init_label = [&](UILabel & label, AssetId & ro_id, std::string_view name,
		std::string_view text, float font_size, glm::vec2 origin)
	{
		label.Init(asset_manager, text, font_atlas, font_size, origin, UILabel::Align::Center, StoryTextColor);
		ro_id = renderer.CreateRenderObject(
			std::string{ name }, RenderLayer::OverlayForeground, label.GetMeshId(),
			text_pipeline_id, label.GetPipelineData());
		label.SetROId(ro_id);
	};

	init_label(m_title_label, m_title_ro_id, "settings title", "SETTINGS",
		StoryLargeFontSize, glm::vec2{ UIWidth * 0.5f, 220.0f });
	init_label(m_master_label, m_master_ro_id, "settings master", "",
		StoryMediumFontSize, glm::vec2{ UIWidth * 0.5f, 420.0f });
	init_label(m_music_label, m_music_ro_id, "settings music", "",
		StoryMediumFontSize, glm::vec2{ UIWidth * 0.5f, 555.0f });
	init_label(m_sound_effects_label, m_sound_effects_ro_id, "settings sound effects", "",
		StoryMediumFontSize, glm::vec2{ UIWidth * 0.5f, 690.0f });
	init_label(m_back_label, m_back_ro_id, "settings back", "Back",
		StoryMediumFontSize, glm::vec2{ UIWidth * 0.5f, 850.0f });
	init_label(m_help_label, m_help_ro_id, "settings help", "Use [Left] and [Right] to adjust volume",
		StorySmallFontSize, glm::vec2{ UIWidth * 0.5f, 980.0f });

	SetVisible(false);
}

bool SettingsOverlay::Update(Input const & input, GameViewport const & viewport)
{
	if (!m_visible)
		return false;

	if (input.KeyJustPressed(Input::Key::Esc))
		return true;

	const std::optional<glm::vec2> pointer_position = viewport.FramebufferToUI(input.GetMousePos());
	if (pointer_position)
	{
		const bool pointer_moved = !m_has_last_pointer_position
			|| pointer_position->x != m_last_pointer_position.x
			|| pointer_position->y != m_last_pointer_position.y;
		if (pointer_moved)
		{
			const Row hovered = hovered_row(pointer_position);
			if (hovered != Row::None)
				m_selected_row = hovered;
			m_last_pointer_position = *pointer_position;
			m_has_last_pointer_position = true;
		}
	}

	if (input.KeyJustPressed(Input::Key::Up) || input.KeyJustPressed('W'))
	{
		switch (m_selected_row)
		{
		case Row::None: m_selected_row = Row::Master; break;
		case Row::Master: m_selected_row = Row::Back; break;
		case Row::Music: m_selected_row = Row::Master; break;
		case Row::SoundEffects: m_selected_row = Row::Music; break;
		case Row::Back: m_selected_row = Row::SoundEffects; break;
		}
	}
	if (input.KeyJustPressed(Input::Key::Down) || input.KeyJustPressed('S'))
	{
		switch (m_selected_row)
		{
		case Row::None: m_selected_row = Row::Master; break;
		case Row::Master: m_selected_row = Row::Music; break;
		case Row::Music: m_selected_row = Row::SoundEffects; break;
		case Row::SoundEffects: m_selected_row = Row::Back; break;
		case Row::Back: m_selected_row = Row::Master; break;
		}
	}

	if (input.KeyJustPressed(Input::Key::Left) || input.KeyJustPressed('A'))
		adjust_selected(-VolumeStep);
	if (input.KeyJustPressed(Input::Key::Right) || input.KeyJustPressed('D'))
		adjust_selected(VolumeStep);

	if (input.MouseButtonJustPressed(Input::MouseButton::Left))
		m_armed_row = hovered_row(pointer_position);
	if (input.MouseButtonJustReleased(Input::MouseButton::Left))
	{
		const Row released_over = hovered_row(pointer_position);
		if (m_armed_row && *m_armed_row != Row::None && released_over == *m_armed_row)
		{
			m_selected_row = released_over;
			if (released_over == Row::Back)
			{
				m_audio_system->PlaySound(SoundCue::ShortChime);
				return true;
			}
			if (pointer_position)
			{
				if (const std::optional<float> adjustment = clicked_adjustment(released_over, *pointer_position))
					adjust_selected(*adjustment);
			}
		}
		m_armed_row.reset();
	}

	if (((input.KeyJustPressed(Input::Key::Enter) && !input.AltIsDown())
		|| input.KeyJustPressed(Input::Key::Space)) && m_selected_row == Row::Back)
	{
		m_audio_system->PlaySound(SoundCue::ShortChime);
		return true;
	}

	update_labels();
	return false;
}

void SettingsOverlay::SetVisible(bool visible)
{
	m_visible = visible;
	m_selected_row = Row::Master;
	m_armed_row.reset();
	m_has_last_pointer_position = false;

	if (!m_renderer)
		return;

	m_renderer->Show(m_backdrop_ro_id, visible);
	m_renderer->Show(m_title_ro_id, visible);
	m_renderer->Show(m_master_ro_id, visible);
	m_renderer->Show(m_music_ro_id, visible);
	m_renderer->Show(m_sound_effects_ro_id, visible);
	m_renderer->Show(m_back_ro_id, visible);
	m_renderer->Show(m_help_ro_id, visible);
	update_labels();
}

SettingsOverlay::Row SettingsOverlay::hovered_row(std::optional<glm::vec2> pointer_position) const
{
	if (!pointer_position)
		return Row::None;

	auto contains = [&](UILabel const & label)
	{
		const UILabel::Bounds & bounds = label.GetBounds();
		return bounds.is_valid
			&& pointer_position->x >= bounds.min.x && pointer_position->x <= bounds.max.x
			&& pointer_position->y >= bounds.min.y && pointer_position->y <= bounds.max.y;
	};

	if (contains(m_master_label)) return Row::Master;
	if (contains(m_music_label)) return Row::Music;
	if (contains(m_sound_effects_label)) return Row::SoundEffects;
	if (contains(m_back_label)) return Row::Back;
	return Row::None;
}

std::optional<float> SettingsOverlay::clicked_adjustment(Row row, glm::vec2 pointer_position) const
{
	UILabel const * label = nullptr;
	switch (row)
	{
	case Row::Master: label = &m_master_label; break;
	case Row::Music: label = &m_music_label; break;
	case Row::SoundEffects: label = &m_sound_effects_label; break;
	case Row::None:
	case Row::Back:
		return std::nullopt;
	}

	const std::string_view text = label->GetText();
	const std::size_t decrease_index = text.find('<');
	const std::size_t increase_index = decrease_index == std::string_view::npos
		? std::string_view::npos
		: text.find('>', decrease_index + 1);

	auto contains = [&](std::size_t character_index)
	{
		const UILabel::Bounds bounds = label->GetCharacterBounds(character_index);
		return bounds.is_valid
			&& pointer_position.x >= bounds.min.x && pointer_position.x <= bounds.max.x
			&& pointer_position.y >= bounds.min.y && pointer_position.y <= bounds.max.y;
	};

	if (contains(decrease_index)) return -VolumeStep;
	if (contains(increase_index)) return VolumeStep;
	return std::nullopt;
}

void SettingsOverlay::adjust_selected(float amount)
{
	if (!m_audio_system)
		return;

	switch (m_selected_row)
	{
	case Row::None:
		break;
	case Row::Master:
	{
		const float old_volume = m_audio_system->GetMasterVolume();
		m_audio_system->SetMasterVolume(old_volume + amount);
		m_audio_system->PlaySound(SoundCue::ShortChime);
		break;
	}
	case Row::Music:
	{
		const float old_volume = m_audio_system->GetMusicVolume();
		m_audio_system->SetMusicVolume(old_volume + amount);
		m_audio_system->PlaySound(SoundCue::ShortChime);
		break;
	}
	case Row::SoundEffects:
	{
		const float old_volume = m_audio_system->GetSoundEffectsVolume();
		m_audio_system->SetSoundEffectsVolume(old_volume + amount);
		m_audio_system->PlaySound(SoundCue::ShortChime);
		break;
	}
	case Row::Back:
		break;
	}
}

std::string SettingsOverlay::volume_text(std::string_view name, float volume, bool selected)
{
	const int percent = static_cast<int>(std::round(std::clamp(volume, 0.0f, 1.0f) * 100.0f));
	const std::string text = std::string{ name } + "    <  " + std::to_string(percent) + "%  >";
	return selected ? ">  " + text + "  <" : text;
}

void SettingsOverlay::update_labels()
{
	if (!m_audio_system)
		return;

	const glm::vec4 selected_color{ 1.0f, 0.72f, 0.22f, 1.0f };
	m_master_label.SetText(volume_text("Master Volume", m_audio_system->GetMasterVolume(), m_selected_row == Row::Master));
	m_music_label.SetText(volume_text("Music Volume", m_audio_system->GetMusicVolume(), m_selected_row == Row::Music));
	m_sound_effects_label.SetText(volume_text("Sound Effects Volume", m_audio_system->GetSoundEffectsVolume(), m_selected_row == Row::SoundEffects));
	m_back_label.SetText(m_selected_row == Row::Back ? ">  Back  <" : "Back");
	m_master_label.SetTextColor(m_selected_row == Row::Master ? selected_color : StoryTextColor);
	m_music_label.SetTextColor(m_selected_row == Row::Music ? selected_color : StoryTextColor);
	m_sound_effects_label.SetTextColor(m_selected_row == Row::SoundEffects ? selected_color : StoryTextColor);
	m_back_label.SetTextColor(m_selected_row == Row::Back ? selected_color : StoryTextColor);
}
