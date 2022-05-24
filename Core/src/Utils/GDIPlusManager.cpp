#define FULL_WINTARD
#include "Utils/WinHelper.h"
#include "GDIPlusManager.h"

namespace Gdiplus
{
	using std::min;
	using std::max;
}

#include <gdiplus.h>

ULONG_PTR GDIPlusManager::token_ = 0;
int GDIPlusManager::refCount_ = 0;

GDIPlusManager::GDIPlusManager()
{
	if (refCount_++ == 0)
	{
		Gdiplus::GdiplusStartupInput input;
		input.GdiplusVersion = 1;
		input.DebugEventCallback = nullptr;
		input.SuppressBackgroundThread = false;
		Gdiplus::GdiplusStartup(&token_, &input, nullptr);
	}
}

GDIPlusManager::~GDIPlusManager()
{
	if (--refCount_ == 0)
	{
		Gdiplus::GdiplusShutdown(token_);
	}
}
