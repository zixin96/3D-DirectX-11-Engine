#include "DXKeyboard.h"

namespace D3DEngine
{
	bool DXKeyboard::KeyIsPressed(unsigned char keycode) const noexcept
	{
		return keystates_[keycode];
	}

	std::optional<DXKeyboard::Event> DXKeyboard::ReadKey() noexcept
	{
		if (keybuffer_.size() > 0u)
		{
			DXKeyboard::Event e = keybuffer_.front();
			keybuffer_.pop();
			return e;
		}
		return {};
	}

	bool DXKeyboard::KeyIsEmpty() const noexcept
	{
		return keybuffer_.empty();
	}

	std::optional<char> DXKeyboard::ReadChar() noexcept
	{
		if (charbuffer_.size() > 0u)
		{
			unsigned char charcode = charbuffer_.front();
			charbuffer_.pop();
			return charcode;
		}
		return {};
	}

	bool DXKeyboard::CharIsEmpty() const noexcept
	{
		return charbuffer_.empty();
	}

	void DXKeyboard::ClearKey() noexcept
	{
		keybuffer_ = std::queue<Event>();
	}

	void DXKeyboard::ClearChar() noexcept
	{
		charbuffer_ = std::queue<char>();
	}

	void DXKeyboard::Clear() noexcept
	{
		ClearKey();
		ClearChar();
	}

	void DXKeyboard::EnableAutorepeat() noexcept
	{
		autorepeatEnabled_ = true;
	}

	void DXKeyboard::DisableAutorepeat() noexcept
	{
		autorepeatEnabled_ = false;
	}

	bool DXKeyboard::AutorepeatIsEnabled() const noexcept
	{
		return autorepeatEnabled_;
	}

	void DXKeyboard::OnKeyPressed(unsigned char keycode) noexcept
	{
		keystates_[keycode] = true;
		keybuffer_.push(Event(Event::Type::Press, keycode));
		TrimBuffer(keybuffer_);
	}

	void DXKeyboard::OnKeyReleased(unsigned char keycode) noexcept
	{
		keystates_[keycode] = false;
		keybuffer_.push(Event(Event::Type::Release, keycode));
		TrimBuffer(keybuffer_);
	}

	void DXKeyboard::OnChar(char character) noexcept
	{
		charbuffer_.push(character);
		TrimBuffer(charbuffer_);
	}

	void DXKeyboard::ClearState() noexcept
	{
		keystates_.reset();
	}

	template <typename T>
	void DXKeyboard::TrimBuffer(std::queue<T>& buffer) noexcept
	{
		while (buffer.size() > BUFFER_SIZE)
		{
			buffer.pop();
		}
	}
}
