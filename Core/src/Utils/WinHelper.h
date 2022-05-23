// Always include this first!

#pragma once

// target Windows 7 or later
//#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>
// The following #defines disable a bunch of unused windows stuff. If you 
// get weird errors when trying to do some windows stuff, try removing some
// (or all) of these defines (it will increase build time though).
#define NOMINMAX

// #define STRICT

#include <Windows.h>
