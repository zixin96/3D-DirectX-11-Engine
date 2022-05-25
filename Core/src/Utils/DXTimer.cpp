#include "DXTimer.h"

using namespace std::chrono;

DXTimer::DXTimer() noexcept
{
	last_ = steady_clock::now();
}

/**
 * \brief Returns time elapsed since the last time we called Mark() and resets the mark point
 * \return Time elapsed since the last time we called Mark()
 */
float DXTimer::Mark() noexcept
{
	const auto old = last_;
	last_ = steady_clock::now();
	const duration<float> frameTime = last_ - old;
	return frameTime.count();
}

/**
 * \brief Returns time elapsed since the last time we called Mark(), without resetting the mark point
 * \return Time elapsed since the last time we called Mark()
 */
float DXTimer::Peek() const noexcept
{
	return duration<float>(steady_clock::now() - last_).count();
}
