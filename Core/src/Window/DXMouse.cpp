#include "Utils/WinHelper.h"
#include "DXMouse.h"

std::pair<int, int> DXMouse::GetPos() const noexcept
{
	return {x_, y_};
}

int DXMouse::GetPosX() const noexcept
{
	return x_;
}

int DXMouse::GetPosY() const noexcept
{
	return y_;
}

bool DXMouse::IsInWindow() const noexcept
{
	return isInWindow_;
}

bool DXMouse::LeftIsPressed() const noexcept
{
	return leftIsPressed_;
}

bool DXMouse::RightIsPressed() const noexcept
{
	return rightIsPressed_;
}

DXMouse::Event DXMouse::Read() noexcept
{
	if (buffer_.size() > 0u)
	{
		Event e = buffer_.front();
		buffer_.pop();
		return e;
	}
	return Event();
}

void DXMouse::Clear() noexcept
{
	buffer_ = std::queue<Event>();
}

void DXMouse::OnMouseMove(int newx, int newy) noexcept
{
	x_ = newx;
	y_ = newy;

	buffer_.push(Event(Event::Type::Move, *this));
	TrimBuffer();
}

void DXMouse::OnMouseLeave() noexcept
{
	isInWindow_ = false;
	buffer_.push(Event(Event::Type::Leave, *this));
	TrimBuffer();
}

void DXMouse::OnMouseEnter() noexcept
{
	isInWindow_ = true;
	buffer_.push(Event(Event::Type::Enter, *this));
	TrimBuffer();
}

void DXMouse::OnLeftPressed(int x, int y) noexcept
{
	leftIsPressed_ = true;

	buffer_.push(Event(Event::Type::LPress, *this));
	TrimBuffer();
}

void DXMouse::OnLeftReleased(int x, int y) noexcept
{
	leftIsPressed_ = false;

	buffer_.push(Event(Event::Type::LRelease, *this));
	TrimBuffer();
}

void DXMouse::OnRightPressed(int x, int y) noexcept
{
	rightIsPressed_ = true;

	buffer_.push(Event(Event::Type::RPress, *this));
	TrimBuffer();
}

void DXMouse::OnRightReleased(int x, int y) noexcept
{
	rightIsPressed_ = false;

	buffer_.push(Event(Event::Type::RRelease, *this));
	TrimBuffer();
}

void DXMouse::OnWheelUp(int x, int y) noexcept
{
	buffer_.push(Event(Event::Type::WheelUp, *this));
	TrimBuffer();
}

void DXMouse::OnWheelDown(int x, int y) noexcept
{
	buffer_.push(Event(Event::Type::WheelDown, *this));
	TrimBuffer();
}

void DXMouse::TrimBuffer() noexcept
{
	while (buffer_.size() > bufferSize_)
	{
		buffer_.pop();
	}
}

void DXMouse::OnWheelDelta(int x, int y, int delta) noexcept
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
