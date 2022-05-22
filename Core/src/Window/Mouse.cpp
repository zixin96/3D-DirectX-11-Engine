#include "Mouse.h"
#include <Windows.h>

std::pair<int, int> Mouse::GetPos() const noexcept
{
	return {x_, y_};
}

int Mouse::GetPosX() const noexcept
{
	return x_;
}

int Mouse::GetPosY() const noexcept
{
	return y_;
}

bool Mouse::IsInWindow() const noexcept
{
	return isInWindow_;
}

bool Mouse::LeftIsPressed() const noexcept
{
	return leftIsPressed_;
}

bool Mouse::RightIsPressed() const noexcept
{
	return rightIsPressed_;
}

Mouse::Event Mouse::Read() noexcept
{
	if (buffer_.size() > 0u)
	{
		Event e = buffer_.front();
		buffer_.pop();
		return e;
	}
	return Event();
}

void Mouse::Clear() noexcept
{
	buffer_ = std::queue<Event>();
}

void Mouse::OnMouseMove(int newx, int newy) noexcept
{
	x_ = newx;
	y_ = newy;

	buffer_.push(Event(Event::Type::Move, *this));
	TrimBuffer();
}

void Mouse::OnMouseLeave() noexcept
{
	isInWindow_ = false;
	buffer_.push(Event(Event::Type::Leave, *this));
	TrimBuffer();
}

void Mouse::OnMouseEnter() noexcept
{
	isInWindow_ = true;
	buffer_.push(Event(Event::Type::Enter, *this));
	TrimBuffer();
}

void Mouse::OnLeftPressed(int x, int y) noexcept
{
	leftIsPressed_ = true;

	buffer_.push(Event(Event::Type::LPress, *this));
	TrimBuffer();
}

void Mouse::OnLeftReleased(int x, int y) noexcept
{
	leftIsPressed_ = false;

	buffer_.push(Event(Event::Type::LRelease, *this));
	TrimBuffer();
}

void Mouse::OnRightPressed(int x, int y) noexcept
{
	rightIsPressed_ = true;

	buffer_.push(Event(Event::Type::RPress, *this));
	TrimBuffer();
}

void Mouse::OnRightReleased(int x, int y) noexcept
{
	rightIsPressed_ = false;

	buffer_.push(Event(Event::Type::RRelease, *this));
	TrimBuffer();
}

void Mouse::OnWheelUp(int x, int y) noexcept
{
	buffer_.push(Event(Event::Type::WheelUp, *this));
	TrimBuffer();
}

void Mouse::OnWheelDown(int x, int y) noexcept
{
	buffer_.push(Event(Event::Type::WheelDown, *this));
	TrimBuffer();
}

void Mouse::TrimBuffer() noexcept
{
	while (buffer_.size() > bufferSize_)
	{
		buffer_.pop();
	}
}

void Mouse::OnWheelDelta(int x, int y, int delta) noexcept
{
	wheelDeltaCarry_ += delta;
	// generate events for every 120 
	while (wheelDeltaCarry_ >= WHEEL_DELTA)
	{
		wheelDeltaCarry_ -= WHEEL_DELTA;
		OnWheelUp(x, y);
	}
	while (wheelDeltaCarry_ <= -WHEEL_DELTA)
	{
		wheelDeltaCarry_ += WHEEL_DELTA;
		OnWheelDown(x, y);
	}
}
