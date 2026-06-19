// Input.ixx

module;

#include <cstdint>
#include <mutex>
#include <unordered_map>

#include <glm/vec2.hpp>

export module Input;

export class Input
{
public:
	enum class Key : int
	{
		Space   = 32,  // GLFW_KEY_SPACE
		Esc     = 256, // GLFW_KEY_ESCAPE
		Enter   = 257, // GLFW_KEY_ENTER
		Tab     = 258, // GLFW_KEY_TAB
		Backspace = 259, // GLFW_KEY_BACKSPACE
		Delete  = 261, // GLFW_KEY_DELETE
		Right   = 262, // GLFW_KEY_RIGHT
		Left    = 263, // GLFW_KEY_LEFT
		Down    = 264, // GLFW_KEY_DOWN
		Up      = 265, // GLFW_KEY_UP
		LeftShift = 340, // GLFW_KEY_LEFT_SHIFT
		LeftControl = 341, // GLFW_KEY_LEFT_CONTROL
		RightShift = 344, // GLFW_KEY_RIGHT_SHIFT
		RightControl = 345, // GLFW_KEY_RIGHT_CONTROL
	};

	enum class Action : int
	{
		Press   = 1, // GLFW_PRESS
		Release = 0, // GLFW_RELEASE
	};

	enum class Mod : int
	{
		Shift   = 0x1, // GLFW_MOD_SHIFT
		Control = 0x2, // GLFW_MOD_CONTROL
		Alt     = 0x4, // GLFW_MOD_ALT
	};

	enum class MouseButton : int
	{
		Left  = 0, // GLFW_MOUSE_BUTTON_LEFT
		Right = 1, // GLFW_MOUSE_BUTTON_RIGHT
	};

	void SetKey(int key, bool pressed);
	void SetMouseButton(int button, bool pressed);
	void SetMousePos(float x, float y);
	void NewFrame();

	bool KeyIsDown(int key) const;
	bool KeyIsDown(Key key) const;
	bool KeyJustPressed(int key) const;
	bool KeyJustPressed(Key key) const;
	bool KeyJustReleased(int key) const;
	bool KeyJustReleased(Key key) const;
	bool MouseButtonIsDown(MouseButton button) const;
	bool MouseButtonJustPressed(MouseButton button) const;
	bool MouseButtonJustReleased(MouseButton button) const;
	glm::vec2 GetMousePos() const { return m_mouse_pos; }

private:
	static constexpr std::uint8_t StatePressed  = 0x1; // Pressed this frame
	static constexpr std::uint8_t StateDown     = 0x2; // Down for 1 or more frames
	static constexpr std::uint8_t StateReleased = 0x4; // Released this frame

	static void apply_pending_state(
		std::unordered_map<int, std::uint8_t> const & pending_state,
		std::unordered_map<int, std::uint8_t> & state);
	static void prepare_state_for_new_frame(std::unordered_map<int, std::uint8_t> & state);

private:
	mutable std::mutex m_pending_mutex;
	std::unordered_map<int, std::uint8_t> m_pending_state;
	std::unordered_map<int, std::uint8_t> m_pending_mouse_button_state;
	glm::vec2 m_pending_mouse_pos{ 0.0f };
	std::unordered_map<int, std::uint8_t> m_key_state;
	std::unordered_map<int, std::uint8_t> m_mouse_button_state;
	glm::vec2 m_mouse_pos{ 0.0f };
};

void Input::SetKey(int key, bool pressed)
{
	std::lock_guard lock(m_pending_mutex);
	if (pressed)
		m_pending_state[key] |= StatePressed;
	else
		m_pending_state[key] |= StateReleased;
}

void Input::SetMouseButton(int button, bool pressed)
{
	std::lock_guard lock(m_pending_mutex);
	if (pressed)
		m_pending_mouse_button_state[button] |= StatePressed;
	else
		m_pending_mouse_button_state[button] |= StateReleased;
}

void Input::SetMousePos(float x, float y)
{
	std::lock_guard lock(m_pending_mutex);
	m_pending_mouse_pos = glm::vec2{ x, y };
}

void Input::NewFrame()
{
	prepare_state_for_new_frame(m_key_state);
	prepare_state_for_new_frame(m_mouse_button_state);

	std::lock_guard lock(m_pending_mutex);
	apply_pending_state(m_pending_state, m_key_state);
	apply_pending_state(m_pending_mouse_button_state, m_mouse_button_state);
	m_mouse_pos = m_pending_mouse_pos;
	m_pending_state.clear();
	m_pending_mouse_button_state.clear();
}

bool Input::KeyIsDown(int key) const
{
	auto iter = m_key_state.find(key);
	return iter != m_key_state.end() && iter->second & StateDown;
}

bool Input::KeyIsDown(Key key) const
{
	return KeyIsDown(static_cast<int>(key));
}

bool Input::KeyJustPressed(int key) const
{
	auto iter = m_key_state.find(key);
	return iter != m_key_state.end() && iter->second & StatePressed;
}

bool Input::KeyJustPressed(Key key) const
{
	return KeyJustPressed(static_cast<int>(key));
}

bool Input::KeyJustReleased(int key) const
{
	auto iter = m_key_state.find(key);
	return iter != m_key_state.end() && iter->second & StateReleased;
}

bool Input::KeyJustReleased(Key key) const
{
	return KeyJustReleased(static_cast<int>(key));
}

bool Input::MouseButtonIsDown(MouseButton button) const
{
	auto iter = m_mouse_button_state.find(static_cast<int>(button));
	return iter != m_mouse_button_state.end() && iter->second & StateDown;
}

bool Input::MouseButtonJustPressed(MouseButton button) const
{
	auto iter = m_mouse_button_state.find(static_cast<int>(button));
	return iter != m_mouse_button_state.end() && iter->second & StatePressed;
}

bool Input::MouseButtonJustReleased(MouseButton button) const
{
	auto iter = m_mouse_button_state.find(static_cast<int>(button));
	return iter != m_mouse_button_state.end() && iter->second & StateReleased;
}

void Input::apply_pending_state(
	std::unordered_map<int, std::uint8_t> const & pending_state,
	std::unordered_map<int, std::uint8_t> & state)
{
	for (auto const & [key, pending] : pending_state)
	{
		std::uint8_t & cur_state = state[key];
		if (pending & StatePressed)
			cur_state = StatePressed | StateDown;
		if (pending & StateReleased)
			cur_state = (cur_state | StateReleased) & ~StateDown;
	}
}

void Input::prepare_state_for_new_frame(std::unordered_map<int, std::uint8_t> & state)
{
	std::erase_if(state, [](auto const & pair) { return pair.second & StateReleased; });
	for (auto & [key, cur_state] : state)
		cur_state = cur_state & ~StatePressed;
}
