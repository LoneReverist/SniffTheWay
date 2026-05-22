// Input.ixx

module;

#include <cstdint>
#include <mutex>
#include <unordered_map>

export module Input;

export class Input
{
public:
	enum class Key : int
	{
		Space   = 32,  // GLFW_KEY_SPACE
		Esc     = 256, // GLFW_KEY_ESCAPE
		Enter   = 257, // GLFW_KEY_ENTER
		Right   = 262, // GLFW_KEY_RIGHT
		Left    = 263, // GLFW_KEY_LEFT
		Down    = 264, // GLFW_KEY_DOWN
		Up      = 265, // GLFW_KEY_UP
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

	void SetKey(int key, bool pressed);
	void NewFrame();

	bool KeyIsDown(int key) const;
	bool KeyIsDown(Key key) const;
	bool KeyJustPressed(int key) const;
	bool KeyJustPressed(Key key) const;
	bool KeyJustReleased(int key) const;
	bool KeyJustReleased(Key key) const;

private:
	static constexpr std::uint8_t StatePressed  = 0x1; // Pressed this frame
	static constexpr std::uint8_t StateDown     = 0x2; // Down for 1 or more frames
	static constexpr std::uint8_t StateReleased = 0x4; // Released this frame

private:
	mutable std::mutex m_pending_mutex;
	std::unordered_map<int, std::uint8_t> m_pending_state;
	std::unordered_map<int, std::uint8_t> m_key_state;
};

void Input::SetKey(int key, bool pressed)
{
	std::lock_guard lock(m_pending_mutex);
	if (pressed)
		m_pending_state[key] |= StatePressed;
	else
		m_pending_state[key] |= StateReleased;
}

void Input::NewFrame()
{
	std::erase_if(m_key_state, [](auto const & pair) { return pair.second & StateReleased; });
	for (auto & [key, state] : m_key_state)
		state = state & ~StatePressed;

	std::lock_guard lock(m_pending_mutex);
	for (auto const & [key, pending_state] : m_pending_state)
	{
		std::uint8_t & state = m_key_state[key];
		if (pending_state & StatePressed)
			state = StatePressed | StateDown;
		if (pending_state & StateReleased)
			state = (state | StateReleased) & ~StateDown;
	}
	m_pending_state.clear();
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
