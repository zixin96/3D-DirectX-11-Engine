#pragma once

/**
 * \brief A class that encapsulates access to the keyboard input
 */
class DXKeyboard
{
	// "friend" the Window will gives Window access to Keyboard's private functions
	friend class DXWindow;
public:
	class Event
	{
	public:
		enum class Type
		{
			Press,
			Release,
			Invalid
		};

	private:
		Type type_;
		unsigned char code_;
	public:
		Event() noexcept
			:
			type_(Type::Invalid),
			code_(0u)
		{
		}

		Event(Type type, unsigned char code) noexcept
			:
			type_(type),
			code_(code)
		{
		}

		bool IsPress() const noexcept
		{
			return type_ == Type::Press;
		}

		bool IsRelease() const noexcept
		{
			return type_ == Type::Release;
		}

		bool IsValid() const noexcept
		{
			return type_ != Type::Invalid;
		}

		unsigned char GetCode() const noexcept
		{
			return code_;
		}
	};


	DXKeyboard() = default;

	DXKeyboard(const DXKeyboard&) = delete;
	DXKeyboard& operator=(const DXKeyboard&) = delete;

	// key event stuff

	// is the key (represented by keycode) being pressed? 
	bool KeyIsPressed(unsigned char keycode) const noexcept;

	// pull an key event off the event queue
	Event ReadKey() noexcept;

	// check if there are any key events in the event queue
	bool KeyIsEmpty() const noexcept;

	// flush the key event queue
	void ClearKey() noexcept;

	// char event (used for text input) stuff

	char ReadChar() noexcept;
	bool CharIsEmpty() const noexcept;
	void ClearChar() noexcept;

	// flush both char and key queues
	void Clear() noexcept;

	// autorepeat control
	void EnableAutorepeat() noexcept;
	void DisableAutorepeat() noexcept;
	bool AutorepeatIsEnabled() const noexcept;

private:
	void OnKeyPressed(unsigned char keycode) noexcept;
	void OnKeyReleased(unsigned char keycode) noexcept;
	void OnChar(char character) noexcept;
	void ClearState() noexcept;

	// templated b/c we want it to apply to both event and char queue
	template <typename T>
	static void TrimBuffer(std::queue<T>& buffer) noexcept;

	static constexpr unsigned int NUM_KEYS = 256u;
	static constexpr unsigned int BUFFER_SIZE = 16u;
	bool autorepeatEnabled_ = false;

	// this bit field is used for Keyboard state tracking/querying
	std::bitset<NUM_KEYS> keystates_;

	// Keyboard event queue w/ Event class
	std::queue<Event> keybuffer_;

	// Keyboard text stream queue for WM_CHAR
	std::queue<char> charbuffer_;
};
