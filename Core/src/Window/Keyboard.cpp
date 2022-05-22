#include "Keyboard.h"

bool Keyboard::KeyIsPressed(unsigned char keycode) const noexcept
{
	return keystates_[keycode];
}

Keyboard::Event Keyboard::ReadKey() noexcept
{
	if (keybuffer_.size() > 0u)
	{
		Event e = keybuffer_.front();
		keybuffer_.pop();
		return e;
	}
	return Event();
}

bool Keyboard::KeyIsEmpty() const noexcept
{
	return keybuffer_.empty();
}

char Keyboard::ReadChar() noexcept
{
	if (charbuffer_.size() > 0u)
	{
		unsigned char charcode = charbuffer_.front();
		charbuffer_.pop();
		return charcode;
	}
	return 0;
}

bool Keyboard::CharIsEmpty() const noexcept
{
	return charbuffer_.empty();
}

void Keyboard::ClearKey() noexcept
{
	keybuffer_ = std::queue<Event>();
}

void Keyboard::ClearChar() noexcept
{
	charbuffer_ = std::queue<char>();
}

void Keyboard::Clear() noexcept
{
	ClearKey();
	ClearChar();
}

void Keyboard::EnableAutorepeat() noexcept
{
	autorepeatEnabled_ = true;
}

void Keyboard::DisableAutorepeat() noexcept
{
	autorepeatEnabled_ = false;
}

bool Keyboard::AutorepeatIsEnabled() const noexcept
{
	return autorepeatEnabled_;
}

void Keyboard::OnKeyPressed(unsigned char keycode) noexcept
{
	keystates_[keycode] = true;
	keybuffer_.push(Event(Event::Type::Press, keycode));
	TrimBuffer(keybuffer_);
}

void Keyboard::OnKeyReleased(unsigned char keycode) noexcept
{
	keystates_[keycode] = false;
	keybuffer_.push(Event(Event::Type::Release, keycode));
	TrimBuffer(keybuffer_);
}

void Keyboard::OnChar(char character) noexcept
{
	charbuffer_.push(character);
	TrimBuffer(charbuffer_);
}

void Keyboard::ClearState() noexcept
{
	keystates_.reset();
}

template <typename T>
void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept
{
	while (buffer.size() > BUFFER_SIZE)
	{
		buffer.pop();
	}
}
