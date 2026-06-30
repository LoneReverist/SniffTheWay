// UIButton.ixx

module;

#include <optional>

#include <glm/vec2.hpp>

export module UIButton;

import AssetManager;
import AssetPool;
import Input;
import Texture2dPipeline;
import UIImage;
import Vertex;

export class UIButton
{
public:
	struct Frames
	{
		UIImage::UVBounds normal;
		UIImage::UVBounds hovered;
		UIImage::UVBounds pressed;
	};

	UIButton() = default;

	void Init(
		AssetManager & asset_manager,
		AssetId texture_id,
		glm::vec2 center,
		glm::vec2 size,
		Frames frames);
	void Update(Input const & input, std::optional<glm::vec2> pointer_position);

	bool WasActivated() const { return m_was_activated; }
	MeshId<TextureVertex2d> GetMeshId() const { return m_image.GetMeshId(); }
	Texture2dPipeline::ObjectData const & GetPipelineData() const { return m_image.GetPipelineData(); }

private:
	enum class State
	{
		Normal,
		Hovered,
		Pressed,
	};

	void set_state(State state);

private:
	UIImage m_image;
	Frames m_frames;
	State m_state = State::Normal;
	bool m_is_armed = false;
	bool m_was_activated = false;
};

void UIButton::Init(
	AssetManager & asset_manager,
	AssetId texture_id,
	glm::vec2 center,
	glm::vec2 size,
	Frames frames)
{
	m_frames = frames;
	m_image.Init(asset_manager, texture_id, center, size, m_frames.normal);
}

void UIButton::Update(Input const & input, std::optional<glm::vec2> pointer_position)
{
	m_was_activated = false;
	const bool is_hovered = pointer_position.has_value()
		&& m_image.GetBounds().Contains(pointer_position.value());

	if (input.MouseButtonJustPressed(Input::MouseButton::Left) && is_hovered)
		m_is_armed = true;

	if (input.MouseButtonJustReleased(Input::MouseButton::Left))
	{
		m_was_activated = m_is_armed && is_hovered;
		m_is_armed = false;
	}

	if (m_is_armed && input.MouseButtonIsDown(Input::MouseButton::Left) && is_hovered)
		set_state(State::Pressed);
	else if (is_hovered)
		set_state(State::Hovered);
	else
		set_state(State::Normal);
}

void UIButton::set_state(State state)
{
	if (m_state == state)
		return;

	m_state = state;
	switch (m_state)
	{
	case State::Normal:
		m_image.SetUVBounds(m_frames.normal);
		break;
	case State::Hovered:
		m_image.SetUVBounds(m_frames.hovered);
		break;
	case State::Pressed:
		m_image.SetUVBounds(m_frames.pressed);
		break;
	}
}
