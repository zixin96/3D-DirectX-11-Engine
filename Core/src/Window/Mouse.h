#pragma once

/**
 * \brief A class that encapsulates access to the mouse input
 */
class Mouse
{
	friend class DXWindow;
public:
	class Event
	{
	public:
		enum class Type
		{
			LPress,
			LRelease,
			RPress,
			RRelease,
			WheelUp,
			WheelDown,
			Move,
			// when the mouse enters the client region of the window
			Enter,
			// when the mouse leaves the client region of the window
			Leave,
			Invalid
		};

	private:
		Type type_;
		bool leftIsPressed_;
		bool rightIsPressed_;
		int x_;
		int y_;
	public:
		Event() noexcept
			:
			type_(Type::Invalid),
			leftIsPressed_(false),
			rightIsPressed_(false),
			x_(0),
			y_(0)
		{
		}

		Event(Type type, const Mouse& parent) noexcept
			:
			type_(type),
			leftIsPressed_(parent.leftIsPressed_),
			rightIsPressed_(parent.rightIsPressed_),
			x_(parent.x_),
			y_(parent.y_)
		{
		}

		bool IsValid() const noexcept
		{
			return type_ != Type::Invalid;
		}

		Type GetType() const noexcept
		{
			return type_;
		}

		std::pair<int, int> GetPos() const noexcept
		{
			return {x_, y_};
		}

		int GetPosX() const noexcept
		{
			return x_;
		}

		int GetPosY() const noexcept
		{
			return y_;
		}

		bool LeftIsPressed() const noexcept
		{
			return leftIsPressed_;
		}

		bool RightIsPressed() const noexcept
		{
			return rightIsPressed_;
		}
	};

public:
	Mouse() = default;

	Mouse(const Mouse&) = delete;
	Mouse& operator=(const Mouse&) = delete;

	std::pair<int, int> GetPos() const noexcept;
	int GetPosX() const noexcept;
	int GetPosY() const noexcept;
	bool IsInWindow() const noexcept;
	bool LeftIsPressed() const noexcept;
	bool RightIsPressed() const noexcept;
	Event Read() noexcept;

	bool IsEmpty() const noexcept
	{
		return buffer_.empty();
	}

	void Clear() noexcept;
private:
	void OnMouseMove(int x, int y) noexcept;
	void OnMouseLeave() noexcept;
	void OnMouseEnter() noexcept;
	void OnLeftPressed(int x, int y) noexcept;
	void OnLeftReleased(int x, int y) noexcept;
	void OnRightPressed(int x, int y) noexcept;
	void OnRightReleased(int x, int y) noexcept;
	void OnWheelUp(int x, int y) noexcept;
	void OnWheelDown(int x, int y) noexcept;
	void TrimBuffer() noexcept;
	void OnWheelDelta(int x, int y, int delta) noexcept;
private:
	static constexpr unsigned int bufferSize_ = 16u;

	// cursor position
	int x_;
	int y_;

	bool leftIsPressed_ = false;
	bool rightIsPressed_ = false;
	bool isInWindow_ = false;
	int wheelDeltaCarry_ = 0;
	std::queue<Event> buffer_;
};
