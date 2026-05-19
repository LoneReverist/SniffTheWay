// Input.ixx

module;

#include <mutex>
#include <unordered_set>

export module Input;

export class Input
{
public:
	enum class Key : int
	{
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

	void SetKey(int key, bool pressed)
	{
		std::lock_guard lock(m_key_state_mutex);
		if (pressed)
			m_key_state.insert(key);
		else
			m_key_state.erase(key);
	}

	bool KeyIsPressed(int key) const
	{
		std::lock_guard lock(m_key_state_mutex);
		return m_key_state.contains(key);
	}

	bool KeyIsPressed(Key key) const
	{
		return KeyIsPressed(static_cast<int>(key));
	}

private:
	mutable std::mutex m_key_state_mutex;
	std::unordered_set<int> m_key_state;
};
