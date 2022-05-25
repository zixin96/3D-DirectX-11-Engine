#include "App.h"
#include "AppAssortment.h"
#include "Window/DXWindow.h"

// In this demo, 

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	try
	{
		// App{}.Go();
		AppAssortment{}.Go();
	}
	// catch engine Exception
	catch (const DXException& e)
	{
		// the first parameter indicates that this message box doesn't have a "parent" window
		MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	// then, catch standard exception
	catch (const std::exception& e)
	{
		MessageBox(nullptr, e.what(), "3D Game Programming Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	// WTF??? is this exception?
	catch (...)
	{
		MessageBox(nullptr, "No details available", "3D Game Programming Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}
