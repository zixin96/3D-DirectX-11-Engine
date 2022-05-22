#pragma once
#include "Utils/WinHelper.h"
#include "Debug/DXException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
#include <optional>
#include <memory>

/**
 * \brief A class that encapsulates access to Win32 windows
 */
class Window
{
public:
	class Exception : public DXException
	{
		using DXException::DXException;
	public:
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
	};

	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr_;
	};

	class NoGfxException : public Exception
	{
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;

		// Note: there is no HRESULT associated with this kind of exception
	};

private:
	/**
	 * \brief A private singleton class that manages registration/cleanup of window class
	 */
	class WindowClass
	{
	public:
		static const char* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();

		// since WindowClass is a singleton, we disallow copying
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete;

		static constexpr const char* wndClassName_ = "Direct3D Engine Window";

		// since we use singleton for window class, we declare a static instance for this class,
		// which will be created when the program starts
		static WindowClass wndClass_;

		HINSTANCE hInst_;
	};

public:
	Window(int width, int height, const char* name);
	~Window();

	// assume we only have one window class, so we disallow copying
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	void SetTitle(const std::string& title);
	static std::optional<int> ProcessMessages();
	Graphics& GetGraphics();

	Keyboard kbd_;
	Mouse mouse_;
private:
	// since we cannot pass member function to Win32 directly,
	// we need to setup the following 2 static functions to pass messages to our member function
	static LRESULT WINAPI HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT WINAPI HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	// the actual message handler (a member function)
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	int width_;
	int height_;
	HWND hWnd_;

	// use unique pointer to defer Graphics creation
	// (b/c it can only be created after the window is initialized)
	std::unique_ptr<Graphics> pGfx_;
};

