#pragma once
#define FULL_WINTARD
#include "Utils/WinHelper.h"

/**
 * \brief This Windows Message Logger is used to log all messages coming into the window procedure
 */
class WindowsMessageMap
{
public:
	WindowsMessageMap();
	std::string operator()(DWORD msg, LPARAM lp, WPARAM wp) const;
private:
	// map Window Message ID to a string that represents the message
	std::unordered_map<DWORD, std::string> map;
};
