#pragma once
#include "Utils/WinHelper.h"
#include "Debug/DXException.h"
#include "Debug/DxgiInfoManager.h"
#include "Debug/ConditionalNoexcept.h"

#include <d3d11.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

class Bindable;

/**
 * \brief This class wraps all necessary D3D objects for rendering
 */
class Graphics
{
	// Bindable now have access to private member of Graphics class
	friend class Bindable;
public:
	Graphics(HWND hWnd, int width, int height);

	// we don't want to copy/move Graphics object
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;

	~Graphics();

	void DrawIndexed(UINT count) noxnd;

	void SetProjection(DirectX::FXMMATRIX proj) noexcept;
	DirectX::XMMATRIX GetProjection() const noexcept;

	void SetCamera(DirectX::FXMMATRIX cam) noexcept;
	DirectX::XMMATRIX GetCamera() const noexcept;

	void BeginFrame(float red, float green, float blue) noexcept;
	void EndFrame();
	void ClearBuffer(float red, float green, float blue) noexcept;

	void EnableImgui() noexcept;
	void DisableImgui() noexcept;
	bool IsImguiEnabled() const noexcept;

private:
	bool imguiEnabled_ = true;

	DirectX::XMMATRIX projection_;
	DirectX::XMMATRIX camera_;

#ifdef DX_DEBUG
	DxgiInfoManager infoManager_;
#endif

	Microsoft::WRL::ComPtr<ID3D11Device> pDevice_;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext_;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTargetView_;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView_;

public:
	// Basic graphics Exception
	class Exception : public DXException
	{
		using DXException::DXException;
	};

	// Graphics exception with HRESULT
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		// Get Debug Layer message
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr_;
		std::string info_;
	};

	// A specialized HrException
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	};

	// Exception for functions that don't return HRESULT
	class InfoException : public Exception
	{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info_;
	};
};
