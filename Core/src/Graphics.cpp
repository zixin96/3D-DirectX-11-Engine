#include "Graphics.h"

#include "Debug/GraphicsThrowMacros.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Debug/dxerr.h"

namespace D3DEngine
{
	// prefer changing namespace in cpp files
	namespace wrl = Microsoft::WRL;
	namespace dx = DirectX;

	Graphics::Graphics(HWND hWnd, int width, int height)
	{
		// ---------------Swap Chain and Device Creation Stage--------------------------
		// Describes a swap chain
		DXGI_SWAP_CHAIN_DESC sd = {};
		// describes the backbuffer display mode
		sd.BufferDesc.Width  = width;
		sd.BufferDesc.Height = height;
		// layout of the pixels
		sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		// since we're not doing fullscreen, leave refresh rate as 0
		sd.BufferDesc.RefreshRate.Numerator   = 0;
		sd.BufferDesc.RefreshRate.Denominator = 0;
		// no scaling required
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		// no ordering required
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		// describes multi-sampling parameters
		// No anti-aliasing for now
		sd.SampleDesc.Count   = 1;
		sd.SampleDesc.Quality = 0;
		// use this buffer as render target
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		// Use double-buffering to minimize latency
		sd.BufferCount = 2;
		// specify our window handle
		sd.OutputWindow = hWnd;
		sd.Windowed     = TRUE;
		// use a flip effect
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.Flags      = 0;

		UINT swapCreateFlags = 0u;

		#ifdef DX_DEBUG
		swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		// since all debugging macros expect a HRESULT in local scope,
		// remember to create a HRESULT before adding those macros
		HRESULT hr;

		// Creates a device that represents the display adapter and
		// a swap chain used for rendering
		GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
			               // A pointer to the video adapter to use when creating a device
			               // Pass NULL to use the default adapter
			               nullptr,
			               // the driver type to create
			               D3D_DRIVER_TYPE_HARDWARE,
			               // A handle to a DLL that implements a software rasterizer
			               nullptr,
			               // Add debug layer if in debug mode
			               swapCreateFlags,
			               // no specific feature levels specified
			               nullptr,
			               // no feature levels
			               0,
			               // targeting D3D11
			               D3D11_SDK_VERSION,
			               // swap chain descriptor
			               &sd,
			               &pSwapChain_,
			               &pDevice_,
			               // we don't need to determine which feature level is supported
			               nullptr,
			               &pDeviceContext_
		               ));
		// ------------End Swap Chain and Device Creation Stage--------------------------


		// ---------------Retrieve Render Target View--------------------------
		// gain access to texture subresource in swap chain (back buffer)
		wrl::ComPtr<ID3D11Resource> pBackBuffer;

		GFX_THROW_INFO(pSwapChain_->GetBuffer(
			               // the index of the buffer we're getting (back buffer has index 0)
			               0,
			               // The type of interface used to manipulate the buffer
			               __uuidof(ID3D11Resource),
			               // A pointer to a back-buffer interface
			               &pBackBuffer));

		GFX_THROW_INFO(pDevice_->CreateRenderTargetView(
			               // the buffer we want to get the view on
			               pBackBuffer.Get(),
			               // no additional configuration yet
			               nullptr,
			               // output render view handle
			               &pRenderTargetView_
		               ));
		// ------------End Retrieve Render Target View--------------------------


		// ------------Depth stencil Creation Stage--------------------------
		// Describes depth-stencil state
		D3D11_DEPTH_STENCIL_DESC dsDesc = {
			// Enable depth testing
			.DepthEnable = TRUE,
			// Identify a portion of the depth-stencil buffer that can be modified by depth data
			.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
			// A function that compares depth data against existing depth data
			.DepthFunc = D3D11_COMPARISON_LESS,
		};
		wrl::ComPtr<ID3D11DepthStencilState> pDSState;
		GFX_THROW_INFO(pDevice_->CreateDepthStencilState(&dsDesc, &pDSState));

		// bind depth state
		pDeviceContext_->OMSetDepthStencilState(pDSState.Get(), 1u);

		// create depth stensil texture
		wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
		D3D11_TEXTURE2D_DESC         descDepth = {
			// These should match those of the swap chain
			.Width = (UINT)width,
			.Height = (UINT)height,
			.MipLevels = 1u,
			.ArraySize = 1u,
			// special format for depth values
			.Format = DXGI_FORMAT_D32_FLOAT,
			.SampleDesc = {.Count = 1, .Quality = 0},
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_DEPTH_STENCIL,
		};

		GFX_THROW_INFO(pDevice_->CreateTexture2D(
			               &descDepth,
			               // no initial data b/c it's generated every frame as depth information
			               nullptr,
			               &pDepthStencil));

		// create view of depth stencil texture
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {
			// Resource data format
			.Format = DXGI_FORMAT_D32_FLOAT,
			// Type of resource 
			.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
			.Texture2D = {.MipSlice = 0u},
		};

		GFX_THROW_INFO(pDevice_->CreateDepthStencilView(
			               pDepthStencil.Get(), &descDSV, &pDepthStencilView_));
		// ---------End Depth stencil Creation Stage--------------------------


		// ---------Bind Render target and Depth Stencil Stage--------------------------
		pDeviceContext_->OMSetRenderTargets(1u, pRenderTargetView_.GetAddressOf(), pDepthStencilView_.Get());
		// -------End Bind Render target and Depth Stencil Stage--------------------------


		// -------------------------Viewport Stage----------------------------------
		D3D11_VIEWPORT vp = {
			// X position of the left hand side of the viewport
			.TopLeftX = 0,
			// Y position of the top of the viewport
			.TopLeftY = 0,
			// Width of the viewport
			.Width = (FLOAT)width,
			// Height of the viewport
			.Height = (FLOAT)height,
			// Minimum depth of the viewport
			.MinDepth = 0,
			// Maximum depth of the viewport
			.MaxDepth = 1,
		};
		// Bind an array of viewports to the rasterizer stage of the pipeline
		pDeviceContext_->RSSetViewports(
		                                // Number of viewports to bind
		                                1u,
		                                // An array of D3D11_VIEWPORT structures to bind to the device
		                                &vp);
		// ----------------------End Viewport Stage----------------------------------

		// init imgui d3d impl
		ImGui_ImplDX11_Init(pDevice_.Get(), pDeviceContext_.Get());
	}

	Graphics::~Graphics()
	{
		// Shutdown DX11
		ImGui_ImplDX11_Shutdown();
	}

	void Graphics::DrawIndexed(UINT count) noxnd
	{
		// Using flip mode means that we have to rebind render targets every frame
		pDeviceContext_->OMSetRenderTargets(1u, pRenderTargetView_.GetAddressOf(), pDepthStencilView_.Get());
		GFX_THROW_INFO_ONLY(pDeviceContext_->DrawIndexed(count, 0u, 0u));
	}

	void Graphics::SetProjection(DirectX::FXMMATRIX proj) noexcept
	{
		projMat_ = proj;
	}

	DirectX::XMMATRIX Graphics::GetProjection() const noexcept
	{
		return projMat_;
	}

	void Graphics::SetCamera(DirectX::FXMMATRIX cam) noexcept
	{
		cameraMat_ = cam;
	}

	DirectX::XMMATRIX Graphics::GetCamera() const noexcept
	{
		return cameraMat_;
	}

	void Graphics::BeginFrame(float red, float green, float blue) noexcept
	{
		// imgui begin frame
		if (imguiEnabled_)
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}

		const float color[] = {red, green, blue, 1.0f};
		pDeviceContext_->ClearRenderTargetView(pRenderTargetView_.Get(), color);
		pDeviceContext_->ClearDepthStencilView(pDepthStencilView_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
	}

	void Graphics::EndFrame()
	{
		// imgui frame end
		if (imguiEnabled_)
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		HRESULT hr;

		// retrieve the latest debug message
		#ifdef DX_DEBUG
		infoManager_.Set();
		#endif

		// present the backbuffer (flip/swap)
		if (FAILED(hr = pSwapChain_->Present(
			           // target frame rate: machine frame rate
			           // PS: 2u means targeting half the machine frame rate
			           1u,
			           // no flags
			           0u)))
		{
			// Present function can give you a special error code
			// (caused by driver crash, overlocking GPU, etc.): 
			if (hr == DXGI_ERROR_DEVICE_REMOVED)
			{
				// this special error code can be retrieved by GetDeviceRemovedReason
				throw GFX_DEVICE_REMOVED_EXCEPT(pDevice_->GetDeviceRemovedReason());
			}

			// throw regular exception 
			throw GFX_EXCEPT(hr);
		}
	}

	void Graphics::EnableImgui() noexcept
	{
		imguiEnabled_ = true;
	}

	void Graphics::DisableImgui() noexcept
	{
		imguiEnabled_ = false;
	}

	bool Graphics::IsImguiEnabled() const noexcept
	{
		return imguiEnabled_;
	}

	void Graphics::ClearBuffer(float red, float green, float blue) noexcept
	{
		const float color[] = {red, green, blue, 1.0f};
		pDeviceContext_->ClearRenderTargetView(pRenderTargetView_.Get(), color);
		pDeviceContext_->ClearDepthStencilView(pDepthStencilView_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
	}

	// -------------------------------------------------------------------------

	Graphics::HrException::HrException(int                      line,
	                                   const char*              file,
	                                   HRESULT                  hr,
	                                   std::vector<std::string> infoMsgs) noexcept
		:
		Exception(line, file),
		hr_(hr)
	{
		// join all info messages with newlines into single string
		for (const auto& m : infoMsgs)
		{
			info_ += m;
			info_.push_back('\n');
		}
		// remove final newline if exists
		if (!info_.empty())
		{
			info_.pop_back();
		}
	}

	const char* Graphics::HrException::what() const noexcept
	{
		std::ostringstream oss;
		oss << GetType() << std::endl
				<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
				<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
				<< "[Error String] " << GetErrorString() << std::endl
				<< "[Description] " << GetErrorDescription() << std::endl;
		if (!info_.empty())
		{
			oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
		}
		oss << GetOriginString();
		whatBuffer_ = oss.str();
		return whatBuffer_.c_str();
	}

	const char* Graphics::HrException::GetType() const noexcept
	{
		return "3D Game Programming Graphics Exception";
	}

	HRESULT Graphics::HrException::GetErrorCode() const noexcept
	{
		return hr_;
	}

	std::string Graphics::HrException::GetErrorString() const noexcept
	{
		return DXGetErrorString(hr_);
	}

	std::string Graphics::HrException::GetErrorDescription() const noexcept
	{
		char buf[512];
		DXGetErrorDescription(hr_, buf, sizeof(buf));
		return buf;
	}

	std::string Graphics::HrException::GetErrorInfo() const noexcept
	{
		return info_;
	}

	// -----------------------------------------------------------------

	const char* Graphics::DeviceRemovedException::GetType() const noexcept
	{
		return "3D Game Programming Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
	}

	// -----------------------------------------------------------------
	Graphics::InfoException::InfoException(int                      line,
	                                       const char*              file,
	                                       std::vector<std::string> infoMsgs) noexcept
		: Exception(line, file)
	{
		// join all info messages with newlines into single string
		for (const auto& m : infoMsgs)
		{
			info_ += m;
			info_.push_back('\n');
		}
		// remove final newline if exists
		if (!info_.empty())
		{
			info_.pop_back();
		}
	}

	const char* Graphics::InfoException::what() const noexcept
	{
		std::ostringstream oss;
		oss << GetType() << std::endl
				<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
		oss << GetOriginString();
		whatBuffer_ = oss.str();
		return whatBuffer_.c_str();
	}

	const char* Graphics::InfoException::GetType() const noexcept
	{
		return "3D Game Programming Graphics Info Exception";
	}

	std::string Graphics::InfoException::GetErrorInfo() const noexcept
	{
		return info_;
	}
}
