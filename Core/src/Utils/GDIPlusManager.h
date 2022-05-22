#pragma once
#include "WinHelper.h"

class GDIPlusManager
{
public:
	GDIPlusManager();
	~GDIPlusManager();
private:
	static ULONG_PTR token_;
	static int refCount_;
};
