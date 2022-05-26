#include "DXWindow.h"
#include "WindowsResource/resource.h"
#include "Debug/WindowsThrowMacros.h"
#include "backends/imgui_impl_win32.h"

// put it here (instructed by IMGUI's developers)
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

DXWindow::HrException::HrException(int line, const char* file, HRESULT hr) noexcept
	:
	Exception(line, file),
	hr_(hr)
{
}

const char* DXWindow::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
			<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
			<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
			<< "[Description] " << GetErrorDescription() << std::endl
			<< GetOriginString();
	whatBuffer_ = oss.str();
	return whatBuffer_.c_str();
}

const char* DXWindow::HrException::GetType() const noexcept
{
	return "3D Game Programming Window HrException";
}

HRESULT DXWindow::HrException::GetErrorCode() const noexcept
{
	return hr_;
}

std::string DXWindow::HrException::GetErrorDescription() const noexcept
{
	return TranslateErrorCode(hr_);
}

// ---------------------------------------------------------------------------------


/**
 * \brief Get a description string from the window error code
 * \param hr A windows error code
 * \return A description string from the error code
 */
std::string DXWindow::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;
	// windows will allocate memory for err string and make our pointer point to it
	DWORD nMsgLen = FormatMessage(
	                              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
	                              FORMAT_MESSAGE_IGNORE_INSERTS,
	                              nullptr,
	                              hr,
	                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                              reinterpret_cast<LPSTR>(&pMsgBuf),
	                              0,
	                              nullptr
	                             );
	// 0 string length returned indicates a failure
	if (nMsgLen == 0)
	{
		return "Unidentified error code";
	}
	// copy error string from windows-allocated buffer to std::string
	std::string errorString = pMsgBuf;
	// free windows buffer
	LocalFree(pMsgBuf);
	return errorString;
}


// ------------------------------------------------------------------------------


const char* DXWindow::NoGfxException::GetType() const noexcept
{
	return "3D Game Programming Window Exception [No Graphics]";
}


// ------------------------------------------------------------------------------


// since we use singleton for window class, we declare a static instance for this class,
// which will be created when the program starts
DXWindow::WindowClass DXWindow::WindowClass::wndClass_;

/**
 * \brief Register WinAPI window class
 */
DXWindow::WindowClass::WindowClass() noexcept
	:
	// retrieve instance handle
	hInst_(GetModuleHandle(nullptr))
{
	WNDCLASSEX wc = {0};
	wc.cbSize     = sizeof(wc);
	wc.style      = CS_OWNDC;

	// register our custom window procedure
	wc.lpfnWndProc = HandleMsgSetup;

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance  = hInst_;
	wc.hIcon      = static_cast<HICON>(LoadImage(
	                                             hInst_, MAKEINTRESOURCE(IDI_ICON1),
	                                             IMAGE_ICON, 32, 32, 0
	                                            ));
	wc.hCursor       = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName  = nullptr;
	wc.lpszClassName = wndClassName_;
	wc.hIconSm       = static_cast<HICON>(LoadImage(
	                                                hInst_, MAKEINTRESOURCE(IDI_ICON1),
	                                                IMAGE_ICON, 16, 16, 0
	                                               ));
	RegisterClassEx(&wc);
}

/**
 * \brief Deregister WinAPI window class
 */
DXWindow::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName_, hInst_);
}

const char* DXWindow::WindowClass::GetName() noexcept
{
	return wndClassName_;
}

HINSTANCE DXWindow::WindowClass::GetInstance() noexcept
{
	return wndClass_.hInst_;
}


// ------------------------------------------------------------------------------

DXWindow::DXWindow(int width, int height, const char* name)
	: width_(width), height_(height)
{
	// specify desired client region size
	RECT wr;
	wr.left   = 100;
	wr.right  = width_ + wr.left;
	wr.top    = 100;
	wr.bottom = height_ + wr.top;

	// calculate the entire window size based on desired client region size
	if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
	{
		throw WND_LAST_EXCEPT();
	}

	// create window & get hWnd
	hWnd_ = CreateWindow(
	                     WindowClass::GetName(),
	                     name,
	                     WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,

	                     // let Windows decide the starting position of the window
	                     CW_USEDEFAULT,
	                     CW_USEDEFAULT,

	                     // width_/height_ of the window
	                     wr.right - wr.left,
	                     wr.bottom - wr.top,

	                     nullptr,
	                     nullptr,
	                     WindowClass::GetInstance(),

	                     // pointer to window-creation data
	                     // we pass "this" in CreateWindow so that we can extract "this" inside WinAPI callback functions
	                     this
	                    );

	// check for error
	if (hWnd_ == nullptr)
	{
		throw WND_LAST_EXCEPT();
	}

	// show window
	ShowWindow(hWnd_,SW_SHOWDEFAULT);
	// Init ImGui Win32 Impl
	ImGui_ImplWin32_Init(hWnd_);
	// create graphics object
	pGfx_ = std::make_unique<Graphics>(hWnd_, width, height);
}

DXWindow::~DXWindow()
{
	ImGui_ImplWin32_Shutdown();
	DestroyWindow(hWnd_);
}

void DXWindow::SetTitle(const std::string& title)
{
	if (SetWindowText(hWnd_, title.c_str()) == 0)
	{
		throw WND_LAST_EXCEPT();
	}
}

std::optional<int> DXWindow::ProcessMessages()
{
	MSG msg;
	// while queue has messages (PeekMessage returns != 0), remove and dispatch them (but do not block on empty queue)
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// check for quit because peekmessage does not signal this via return val
		if (msg.message == WM_QUIT)
		{
			// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
			return (int)msg.wParam;
		}

		// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// return empty optional when not quitting app
	return {};
}

Graphics& DXWindow::Gfx()
{
	if (!pGfx_)
	{
		throw WND_NOGFX_EXCEPT();
	}
	return *pGfx_;
}

// this installation function is only for setting up pointers to our instance on Win32 side
LRESULT WINAPI DXWindow::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// we use WM_NCCREATE to extract our own Window class pointer
	if (msg == WM_NCCREATE)
	{
		// extract ptr to window class ("this") from creation data:

		// retrieve CREATESTRUCTW* from WM_NCCREATE's lParam
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		// use CREATESTRUCTW* to retrieve our own Window class pointer
		DXWindow* const pWnd = static_cast<DXWindow*>(pCreate->lpCreateParams);

		// set WinAPI-managed user data to store ptr to window class
		SetWindowLongPtr(hWnd,GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		// set message proc to normal (non-setup) handler now that setup is finished
		SetWindowLongPtr(hWnd,GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&DXWindow::HandleMsgThunk));
		// forward message to window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	// if we get a message before the WM_NCCREATE message, handle with default handler
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// this function redirects messages to our own member function
LRESULT WINAPI DXWindow::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// retrieve ptr to window class
	DXWindow* const pWnd = reinterpret_cast<DXWindow*>(GetWindowLongPtr(hWnd,GWLP_USERDATA));
	// forward message to window class handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

// this is our actual message handler
LRESULT DXWindow::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// IMGUI (which has the top priority to receive win32 messages) handles the win32 messages here: 
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}

	const auto imio = ImGui::GetIO();

	switch (msg)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
		// we don't want the DefProc to handle this message because
		// we want our destructor to destroy the window, rather than the DefProc,
		// so return 0 instead of break
			return 0;

		// clear keystate when window loses focus to prevent input getting "stuck"
		case WM_KILLFOCUS:
			kbd_.ClearState();
			break;

		case WM_ACTIVATE:
			// confine/free cursor on window to foreground/background if cursor disabled
			if (!cursorEnabled_)
			{
				if (wParam & WA_ACTIVE)
				{
					ConfineCursor();
					HideCursor();
				}
				else
				{
					FreeCursor();
					ShowCursor();
				}
			}
			break;

		/*********** KEYBOARD MESSAGES ***********/
		case WM_KEYDOWN:
		// syskey commands need to be handled to track ALT key (VK_MENU) and F10
		case WM_SYSKEYDOWN:
			// stifle this keyboard message if imgui wants to capture
			if (imio.WantCaptureKeyboard)
			{
				break;
			}

		// filter auto repeat key event 
			if (!(lParam & 0x40000000) || kbd_.AutorepeatIsEnabled())
			{
				kbd_.OnKeyPressed(static_cast<unsigned char>(wParam));
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			// stifle this keyboard message if imgui wants to capture
			if (imio.WantCaptureKeyboard)
			{
				break;
			}

			kbd_.OnKeyReleased(static_cast<unsigned char>(wParam));
			break;
		case WM_CHAR:
			// stifle this keyboard message if imgui wants to capture
			if (imio.WantCaptureKeyboard)
			{
				break;
			}

			kbd_.OnChar(static_cast<unsigned char>(wParam));
			break;
		/*********** END KEYBOARD MESSAGES ***********/

		/************* MOUSE MESSAGES ****************/

		case WM_MOUSEMOVE:
			{
				const POINTS pt = MAKEPOINTS(lParam);

				// cursorless exclusive gets first dibs
				if (!cursorEnabled_)
				{
					if (!mouse_.IsInWindow())
					{
						SetCapture(hWnd);
						mouse_.OnMouseEnter();
						HideCursor();
					}
					break;
				}

				// stifle this keyboard message if imgui wants to capture
				if (imio.WantCaptureMouse)
				{
					break;
				}

				// we support tracking mouse movement even if mouse is outside
				// the client region (provided the left/right button is pressed)

				// mouse move happens inside the client region
				if (pt.x >= 0 && pt.x < width_ && pt.y >= 0 && pt.y < height_)
				{
					// log the mouse move
					mouse_.OnMouseMove(pt.x, pt.y);
					// if the mouse is not previously in the window: 
					if (!mouse_.IsInWindow())
					{
						// capture the mouse: as long as mouse is captured, even if the mouse leaves the window region, we still going to receive mouse move events
						SetCapture(hWnd);
						// log enter event
						mouse_.OnMouseEnter();
					}
				}
				// mouse move happens outside the client region: 
				else
				{
					// if the mouse left or right button is pressed: 
					if (wParam & (MK_LBUTTON | MK_RBUTTON))
					{
						// generate the mouse move, even if we are outside the client region
						mouse_.OnMouseMove(pt.x, pt.y);
					}
					// button up -> release capture / log event for leaving
					else
					{
						ReleaseCapture();
						mouse_.OnMouseLeave();
					}
				}
				break;
			}
		case WM_LBUTTONDOWN:
			{
				SetForegroundWindow(hWnd);
				if (!cursorEnabled_)
				{
					ConfineCursor();
					HideCursor();
				}
				// stifle this mouse message if imgui wants to capture
				if (imio.WantCaptureMouse)
				{
					break;
				}
				const POINTS pt = MAKEPOINTS(lParam);
				mouse_.OnLeftPressed(pt.x, pt.y);
				break;
			}
		case WM_RBUTTONDOWN:
			{
				// stifle this mouse message if imgui wants to capture
				if (imio.WantCaptureMouse)
				{
					break;
				}
				const POINTS pt = MAKEPOINTS(lParam);
				mouse_.OnRightPressed(pt.x, pt.y);
				break;
			}
		case WM_LBUTTONUP:
			{
				// stifle this mouse message if imgui wants to capture
				if (imio.WantCaptureMouse)
				{
					break;
				}
				const POINTS pt = MAKEPOINTS(lParam);
				mouse_.OnLeftReleased(pt.x, pt.y);
				// release mouse_ if outside of window
				if (pt.x < 0 || pt.x >= width_ || pt.y < 0 || pt.y >= height_)
				{
					ReleaseCapture();
					mouse_.OnMouseLeave();
				}
				break;
			}
		case WM_RBUTTONUP:
			{
				// stifle this mouse message if imgui wants to capture
				if (imio.WantCaptureMouse)
				{
					break;
				}
				const POINTS pt = MAKEPOINTS(lParam);
				mouse_.OnRightReleased(pt.x, pt.y);
				// release mouse_ if outside of window
				if (pt.x < 0 || pt.x >= width_ || pt.y < 0 || pt.y >= height_)
				{
					ReleaseCapture();
					mouse_.OnMouseLeave();
				}
				break;
			}
		case WM_MOUSEWHEEL:
			{
				// stifle this mouse message if imgui wants to capture
				if (imio.WantCaptureMouse)
				{
					break;
				}
				const POINTS pt    = MAKEPOINTS(lParam);
				const int    delta = GET_WHEEL_DELTA_WPARAM(wParam);
				mouse_.OnWheelDelta(pt.x, pt.y, delta);
				break;
			}
		/************** END MOUSE MESSAGES **************/
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void DXWindow::EnableCursor()
{
	cursorEnabled_ = true;
	ShowCursor();
	EnableImGuiMouse();
	FreeCursor();
}

void DXWindow::DisableCursor()
{
	cursorEnabled_ = false;
	HideCursor();
	DisableImGuiMouse();
	ConfineCursor();
}

// --------------Cursor Private Helpers---------------------

// if the counter > 0, the cursor will be visible
// o.w. it's hidden

void DXWindow::HideCursor() noxnd
{
	// ShowCursor(FALSE) decrements the counter
	// keep calling ShowCursor(FALSE) until the counter is below 0
	while (::ShowCursor(FALSE) >= 0);
}

void DXWindow::ShowCursor() noxnd
{
	// ShowCursor(TRUE) increments the counter
	// keep calling ShowCursor(TRUE) until the counter is larger than or equal to 0
	while (::ShowCursor(TRUE) < 0);
}

void DXWindow::EnableImGuiMouse() noxnd
{
	ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

void DXWindow::DisableImGuiMouse() noxnd
{
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
}

void DXWindow::ConfineCursor() noxnd
{
	RECT rect;
	GetClientRect(hWnd_, &rect);
	// map two points of the rectangle into screen space
	MapWindowPoints(hWnd_, nullptr, reinterpret_cast<POINT*>(&rect), 2);
	// trap the cursor
	ClipCursor(&rect);
}

void DXWindow::FreeCursor() noxnd
{
	ClipCursor(nullptr);
}
