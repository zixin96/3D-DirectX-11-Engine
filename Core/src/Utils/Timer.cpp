#include "Timer.h"

using namespace std::chrono;

Timer::Timer() noexcept
{
	last_ = steady_clock::now();
}

float Timer::Mark() noexcept
{
	const auto old = last_;
	last_ = steady_clock::now();
	const duration<float> frameTime = last_ - old;
	return frameTime.count();
}

float Timer::Peek() const noexcept
{
	return duration<float>(steady_clock::now() - last_).count();
}
