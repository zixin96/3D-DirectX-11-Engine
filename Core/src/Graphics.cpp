#include "Graphics.h"

#include "Debug/GraphicsThrowMacros.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Debug/dxerr.h"

// prefer changing namespace in cpp files
namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

Graphics::Graphics(HWND hWnd)
{
	// ---------------Swap Chain and Device Creation Stage--------------------------
	// Describes a swap chain
	DXGI_SWAP_CHAIN_DESC sd = {};
	// automatically configure the buffer width/height based on window
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	// No anti-aliasing for now
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	// use this buffer as render target
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// double buffering: set buffer count to 1
	// Note: we change it to 2
	sd.BufferCount = 2;
	// specify our window handle
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	// Note: we change it from DXGI_SWAP_EFFECT_DISCARD to DXGI_SWAP_EFFECT_FLIP_DISCARD (to get rid of error info)
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = 0;

	UINT swapCreateFlags = 0u;

#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// since all debugging macros expect a HRESULT in local scope,
	// remember to create a HRESULT before adding those macros
	HRESULT hr;

	// create device and front/back buffers, and swap chain and rendering context
	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
		// choose default adapter
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		// no software driver needed
		nullptr,
		// Add debug layer if in debug mode
		swapCreateFlags,
		// no specific feature levels specified
		nullptr,
		0,
		// targeting D3D11
		D3D11_SDK_VERSION,
		// swap chain descriptor
		&sd,
		&pSwap_,
		&pDevice_,
		// no feature level
		nullptr,
		&pContext_
	));
	// ------------End Swap Chain and Device Creation Stage--------------------------


	// ---------------Retrieve Render Target View Stage--------------------------
	// gain access to texture subresource in swap chain (back buffer)
	wrl::ComPtr<ID3D11Resource> pBackBuffer;

	GFX_THROW_INFO(pSwap_->GetBuffer(
		// the index of the buffer we're getting (back buffer has index 0)
		0,
		__uuidof(ID3D11Resource),
		&pBackBuffer));

	GFX_THROW_INFO(pDevice_->CreateRenderTargetView(
		// the buffer we want to get the view on
		pBackBuffer.Get(),
		// no additional configuration yet
		nullptr,
		// output render view handle
		&pTarget_
	));
	// ------------End Retrieve Render Target View Stage--------------------------


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
	pContext_->OMSetDepthStencilState(pDSState.Get(), 1u);

	// create depth stensil texture
	wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {
		// These should match those of the swap chain
		.Width = 1600,
		.Height = 1200,
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
		pDepthStencil.Get(), &descDSV, &pDSV_));
	// ---------End Depth stencil Creation Stage--------------------------


	// ---------Bind Render target and Depth Stencil Stage--------------------------
	pContext_->OMSetRenderTargets(1u, pTarget_.GetAddressOf(), pDSV_.Get());
	// -------End Bind Render target and Depth Stencil Stage--------------------------


	// -------------------------Viewport Stage----------------------------------
	D3D11_VIEWPORT vp = {
		// X position of the left hand side of the viewport
		.TopLeftX = 0,
		// Y position of the top of the viewport
		.TopLeftY = 0,
		// Width of the viewport
		.Width = 1600,
		// Height of the viewport
		.Height = 1200,
		// Minimum depth of the viewport
		.MinDepth = 0,
		// Maximum depth of the viewport
		.MaxDepth = 1,
	};
	// Bind an array of viewports to the rasterizer stage of the pipeline
	pContext_->RSSetViewports(
		// Number of viewports to bind
		1u,
		// An array of D3D11_VIEWPORT structures to bind to the device
		&vp);
	// ----------------------End Viewport Stage----------------------------------

	// init imgui d3d impl
	ImGui_ImplDX11_Init(pDevice_.Get(), pContext_.Get());
}

Graphics::~Graphics()
{
	// Shutdown DX11
	ImGui_ImplDX11_Shutdown();
}

void Graphics::DrawIndexed(UINT count) noexcept(!true)
{
	// Using flip mode means that we have to rebind render targets every frame
	pContext_->OMSetRenderTargets(1u, pTarget_.GetAddressOf(), pDSV_.Get());
	GFX_THROW_INFO_ONLY(pContext_->DrawIndexed(count, 0u, 0u));
}

void Graphics::SetProjection(DirectX::FXMMATRIX proj) noexcept
{
	projection_ = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection_;
}

void Graphics::SetCamera(DirectX::FXMMATRIX cam) noexcept
{
	camera_ = cam;
}

DirectX::XMMATRIX Graphics::GetCamera() const noexcept
{
	return camera_;
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
	pContext_->ClearRenderTargetView(pTarget_.Get(), color);
	pContext_->ClearDepthStencilView(pDSV_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
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
#ifndef NDEBUG
	infoManager_.Set();
#endif

	// present the backbuffer (flip/swap)
	if (FAILED(hr = pSwap_->Present(
		// target frame rate: machine frame rate
		// PS: 2u means targeting half the machine frame rate
		1u,
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
	pContext_->ClearRenderTargetView(pTarget_.Get(), color);
	pContext_->ClearDepthStencilView(pDSV_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

/*void Graphics::Draw13_First_Triangle()
{
	HRESULT hr;

	// ----------------------------Vertex Buffer Stage-------------------------------------
	struct Vertex
	{
		float x;
		float y;
	};

	// create vertex buffer (See Markdown for visual)
	const Vertex vertices[] =
	{
		{0.0f, 0.5f},
		{0.5f, -0.5f},
		{-0.5f, -0.5f},
	};

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;

	/#1#/ Describes a buffer resource
	D3D11_BUFFER_DESC bd = {
		// Size of the buffer in bytes
		.ByteWidth = sizeof(vertices),
		// Identify how the buffer is expected to be read from and written to
		.Usage = D3D11_USAGE_DEFAULT,
		// Identify how the buffer will be bound to the pipeline
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		// CPU access flags
		.CPUAccessFlags = 0u,
		// Miscellaneous flags
		.MiscFlags = 0u,
		// The size of each element in the buffer structure (in bytes) when the buffer represents a structured buffer
		.StructureByteStride = sizeof(Vertex),
	};

	// Specifies data for initializing a subresource
	D3D11_SUBRESOURCE_DATA sd = {
		// Pointer to the initialization data
		.pSysMem = vertices,
		// The distance (in bytes) from the beginning of one line of a texture to the next line
		.SysMemPitch = 0u,
		// The distance (in bytes) from the beginning of one depth level to the next
		.SysMemSlicePitch = 0u,
	};

	// Creates a buffer (vertex buffer, index buffer, or shader-constant buffer)
	GFX_THROW_INFO(pDevice_->CreateBuffer(
		// A pointer to a D3D11_BUFFER_DESC structure that describes the buffer
		&bd,
		// A pointer to a D3D11_SUBRESOURCE_DATA structure that describes the initialization data
		&sd,
		// Address of a pointer to the ID3D11Buffer interface for the buffer object created.
		&pVertexBuffer));

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;

	// Bind an array of vertex buffers to the input-assembler stage.
	pContext_->IASetVertexBuffers(
		// The first input slot for binding
		0u,
		// The number of vertex buffers in the array
		1u,
		// A pointer to an array of vertex buffers (Pay special attention here: DO NO USE & OPERATOR!)
		pVertexBuffer_.GetAddressOf(),
		// Pointer to an array of stride values
		&stride,
		// Pointer to an array of offset values
		&offset
	);#1#
	// ----------------------------End Vertex Buffer Stage-------------------------------------


	// ----------------------------Create Shader Stage-------------------------------------
	// create pixel shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	wrl::ComPtr<ID3DBlob> pBlob;
	GFX_THROW_INFO(D3DReadFileToBlob(
		L"Shaders/cso/PixelShader13_First_Triangle.cso",
		&pBlob
	));
	GFX_THROW_INFO(pDevice_->CreatePixelShader(
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		nullptr,
		&pPixelShader));

	// bind pixel shader
	pContext_->PSSetShader(
		pPixelShader.Get(),
		nullptr,
		0);

	// create vertex shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_INFO(D3DReadFileToBlob(
		// A pointer to a constant null-terminated (wide) string that contains the name of the file to read into memory
		L"Shaders/cso/VertexShader13_First_Triangle.cso",
		// output handle for the blob
		&pBlob
	));
	GFX_THROW_INFO(pDevice_->CreateVertexShader(
		// A pointer to the compiled shader
		pBlob->GetBufferPointer(),
		// Size of the compiled vertex shader
		pBlob->GetBufferSize(),
		// A pointer to a class linkage interface
		nullptr,
		// Address of a pointer to a ID3D11VertexShader interface
		&pVertexShader));

	// bind vertex shader
	pContext_->VSSetShader(
		// Pointer to a vertex shader
		pVertexShader.Get(),
		nullptr,
		0);
	// -------------------------End Create Shader Stage-------------------------------------


	// ----------------------------input (vertex) layout (2D position only)---------------------------------
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{
			// The HLSL semantic associated with this element in a shader input-signature
			"Position",
			// The semantic index for the element
			0,
			// The data type of the element data
			DXGI_FORMAT_R32G32_FLOAT,
			// An integer value that identifies the input-assemble
			0,
			// Offset (in bytes) from the start of the vertex
			0,
			// Identifies the input data class for a single input slot
			D3D11_INPUT_PER_VERTEX_DATA,
			// The number of instances to draw using the same per-instance data before advancing in the buffer by one element
			0
		},
	};
	GFX_THROW_INFO(pDevice_->CreateInputLayout(
		// An array of the input-assembler stage input data types
		ied,
		// The number of input-data types in the array of input-elements
		(UINT)std::size(ied),
		// A pointer to the compiled (vertex) shader
		pBlob->GetBufferPointer(),
		// Size of the compiled (vertex) shader
		pBlob->GetBufferSize(),
		&pInputLayout));
	// bind input layout
	pContext_->IASetInputLayout(pInputLayout.Get());
	// -------------------------End input (vertex) layout (2D position only)---------------------------------


	// -------------------------Bind Render Target(how would D3D know where you want to render to otherwise?)---------------------------
	pContext_->OMSetRenderTargets(
		// Number of render targets to bind
		1u,
		// Pointer to an array of ID3D11RenderTargetView that represent the render targets to bind to the device
		pTarget_.GetAddressOf(),
		// Pointer to a ID3D11DepthStencilView that represents the depth-stencil view to bind to the device
		pDSV_.Get());
	// ----------------------End Bind Render Target------------------------------------------------


	// --------------------Set primitive topology to triangle list (groups of 3 vertices)------------------------------------------------
	pContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// --------------------End Set primitive topology to triangle list (groups of 3 vertices)------------------------------------------------


	// -------------------------Viewport Stage----------------------------------
	D3D11_VIEWPORT vp = {
		// X position of the left hand side of the viewport
		.TopLeftX = 0,
		// Y position of the top of the viewport
		.TopLeftY = 0,
		// Width of the viewport
		.Width = 800,
		// Height of the viewport
		.Height = 600,
		// Minimum depth of the viewport
		.MinDepth = 0,
		// Maximum depth of the viewport
		.MaxDepth = 1,
	};
	// Bind an array of viewports to the rasterizer stage of the pipeline
	pContext_->RSSetViewports(
		// Number of viewports to bind
		1u,
		// An array of D3D11_VIEWPORT structures to bind to the device
		&vp);
	// ----------------------End Viewport Stage----------------------------------


	// we only need to wrap Draw calls with GFX_THROW_INFO_ONLY. Others won't provide useful information
	GFX_THROW_INFO_ONLY(pContext_->Draw((UINT)std::size(vertices), 0u));
}

void Graphics::Draw14_15_Pipeline_Experiments_And_DirectXMath(float angle, float x, float y)
{
	// how to use DirectX vector?
	dx::XMVECTOR v = dx::XMVectorSet(3.0f, 3.0f, 0.0f, 0.f);
	// DirectX return the result of a dot product in vector format (scalar is replicated)
	auto result = dx::XMVector4Dot(v, v);
	// extract x component of the resulting vector
	auto xx = dx::XMVectorGetX(result);

	// how to transform vector? 
	auto result2 = dx::XMVector3Transform(v, dx::XMMatrixScaling(1.5f, 0.0f, 0.0f));
	auto xx2 = dx::XMVectorGetX(result2);

	HRESULT hr;

	// ----------------------------Buffer Creation Stage-------------------------------------
	struct Vertex
	{
		// add inner structure to our vertex struct
		// note: this will not affect the pipeline configuration, data is still the same
		struct
		{
			float x;
			float y;
		} pos;

		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;
	};

	// create vertex buffer 
	Vertex vertices[] =
	{
		{0.0f, 0.5f, 255, 0, 0, 0},
		{0.5f, -0.5f, 0, 255, 0, 0},
		{-0.5f, -0.5f, 0, 0, 255, 0},
		{-0.3f, 0.3f, 0, 255, 0, 0},
		{0.3f, 0.3f, 0, 0, 255, 0},
		{0.0f, -0.8f, 255, 0, 0, 0},
	};
	vertices[0].color.g = 255;

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;

	// Describes a buffer resource
	D3D11_BUFFER_DESC bd = {
		// Size of the buffer in bytes
		.ByteWidth = sizeof(vertices),
		// Identify how the buffer is expected to be read from and written to
		.Usage = D3D11_USAGE_DEFAULT,
		// Identify how the buffer will be bound to the pipeline
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		// CPU access flags
		.CPUAccessFlags = 0u,
		// Miscellaneous flags
		.MiscFlags = 0u,
		// The size of each element in the buffer structure (in bytes) when the buffer represents a structured buffer
		.StructureByteStride = sizeof(Vertex),
	};

	// Specifies data for initializing a subresource
	D3D11_SUBRESOURCE_DATA sd = {
		// Pointer to the initialization data
		.pSysMem = vertices,
		// The distance (in bytes) from the beginning of one line of a texture to the next line
		.SysMemPitch = 0u,
		// The distance (in bytes) from the beginning of one depth level to the next
		.SysMemSlicePitch = 0u,
	};

	// Creates a buffer (vertex buffer, index buffer, or shader-constant buffer)
	GFX_THROW_INFO(pDevice_->CreateBuffer(
		// A pointer to a D3D11_BUFFER_DESC structure that describes the buffer
		&bd,
		// A pointer to a D3D11_SUBRESOURCE_DATA structure that describes the initialization data
		&sd,
		// Address of a pointer to the ID3D11Buffer interface for the buffer object created.
		&pVertexBuffer));

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;

	// Bind an array of vertex buffers to the input-assembler stage.
	pContext_->IASetVertexBuffers(
		// The first input slot for binding
		0u,
		// The number of vertex buffers in the array
		1u,
		// A pointer to an array of vertex buffers (Pay special attention here: DO NO USE & OPERATOR!)
		pVertexBuffer.GetAddressOf(),
		// Pointer to an array of stride values
		&stride,
		// Pointer to an array of offset values
		&offset
	);

	// create index buffer
	const unsigned short indices[] =
	{
		0, 1, 2,
		0, 2, 3,
		0, 4, 1,
		2, 1, 5,
	};

	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ibd = {
		.ByteWidth = sizeof(indices),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags = 0u,
		.MiscFlags = 0u,
		.StructureByteStride = sizeof(unsigned short),
	};

	D3D11_SUBRESOURCE_DATA isd = {
		.pSysMem = indices,
	};

	GFX_THROW_INFO(pDevice_->CreateBuffer(&ibd, &isd, &pIndexBuffer));

	// bind index buffer
	pContext_->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

	// create constant buffer for transformation matrix
	/*struct ConstantBuffer
	{
		struct
		{
			float element[4][4];
		} transformation;
	};
	const ConstantBuffer cb =
	{
		{
			(3.0f / 4.0f) * std::cos(angle), std::sin(angle), 0.0f, 0.0f,
			(3.0f / 4.0f) * -std::sin(angle), std::cos(angle), 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		}
	};#1#

	// Using DirectXMath will simply our matrix calculation:
	struct ConstantBuffer
	{
		dx::XMMATRIX transform;
	};
	const ConstantBuffer cb =
	{
		{
			dx::XMMatrixTranspose(dx::XMMatrixRotationZ(angle) *
				dx::XMMatrixScaling(3.0f / 4.0f, 1.0f, 1.0f) *
				dx::XMMatrixTranslation(x, y, 0)
			)
		}
	};

	wrl::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd = {
		.ByteWidth = sizeof(cb),
		// dynamic b/c we update constant buffer every frame
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		// CPU will write to the buffer every frame
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0u,
		// no need to provide a stride for this constant buffer
		.StructureByteStride = 0u,
	};
	D3D11_SUBRESOURCE_DATA csd = {
		.pSysMem = &cb,
	};
	GFX_THROW_INFO(pDevice_->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	// bind constant buffer to vertex shader
	pContext_->VSSetConstantBuffers(
		0u,
		1u,
		pConstantBuffer.GetAddressOf());

	// ----------------------------End Buffer Creation Stage-------------------------------------


	// ----------------------------Create Shader Stage-------------------------------------
	// create pixel shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	wrl::ComPtr<ID3DBlob> pBlob;
	GFX_THROW_INFO(D3DReadFileToBlob(
		L"Shaders/cso/PixelShader14_Pipeline_Experiments.cso",
		&pBlob
	));
	GFX_THROW_INFO(pDevice_->CreatePixelShader(
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		nullptr,
		&pPixelShader));

	// bind pixel shader
	pContext_->PSSetShader(
		pPixelShader.Get(),
		nullptr,
		0);

	// create vertex shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_INFO(D3DReadFileToBlob(
		// A pointer to a constant null-terminated (wide) string that contains the name of the file to read into memory
		L"Shaders/cso/VertexShader14_Pipeline_Experiments.cso",
		// output handle for the blob
		&pBlob
	));
	GFX_THROW_INFO(pDevice_->CreateVertexShader(
		// A pointer to the compiled shader
		pBlob->GetBufferPointer(),
		// Size of the compiled vertex shader
		pBlob->GetBufferSize(),
		// A pointer to a class linkage interface
		nullptr,
		// Address of a pointer to a ID3D11VertexShader interface
		&pVertexShader));

	// bind vertex shader
	pContext_->VSSetShader(
		// Pointer to a vertex shader
		pVertexShader.Get(),
		nullptr,
		0);
	// -------------------------End Create Shader Stage-------------------------------------


	// ----------------------------input (vertex) layout (2D position only)---------------------------------
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{
			// The HLSL semantic associated with this element in a shader input-signature
			"Position",
			// The semantic index for the element
			0,
			// The data type of the element data
			DXGI_FORMAT_R32G32_FLOAT,
			// An integer value that identifies the input-assemble
			0,
			// Offset (in bytes) from the start of the vertex
			0,
			// Identifies the input data class for a single input slot
			D3D11_INPUT_PER_VERTEX_DATA,
			// The number of instances to draw using the same per-instance data before advancing in the buffer by one element
			0
		},
		{
			// The HLSL semantic associated with this element in a shader input-signature
			"Color",
			// The semantic index for the element
			0,
			// The data type of the element data (need to use UNORM)
			DXGI_FORMAT_R8G8B8A8_UNORM,
			// An integer value that identifies the input-assemble
			0,
			// Offset (in bytes) from the start of the vertex
			8u,
			// Identifies the input data class for a single input slot
			D3D11_INPUT_PER_VERTEX_DATA,
			// The number of instances to draw using the same per-instance data before advancing in the buffer by one element
			0
		}
	};
	GFX_THROW_INFO(pDevice_->CreateInputLayout(
		// An array of the input-assembler stage input data types
		ied,
		// The number of input-data types in the array of input-elements
		(UINT)std::size(ied),
		// A pointer to the compiled (vertex) shader
		pBlob->GetBufferPointer(),
		// Size of the compiled (vertex) shader
		pBlob->GetBufferSize(),
		&pInputLayout));
	// bind input layout
	pContext_->IASetInputLayout(pInputLayout.Get());
	// -------------------------End input (vertex) layout (2D position only)---------------------------------


	// -------------------------Bind Render Target(how would D3D know where you want to render to otherwise?)---------------------------
	pContext_->OMSetRenderTargets(
		// Number of render targets to bind
		1u,
		// Pointer to an array of ID3D11RenderTargetView that represent the render targets to bind to the device
		pTarget_.GetAddressOf(),
		// Pointer to a ID3D11DepthStencilView that represents the depth-stencil view to bind to the device
		pDSV_.Get());
	// ----------------------End Bind Render Target------------------------------------------------


	// --------------------Set primitive topology to triangle list (groups of 3 vertices)------------------------------------------------
	pContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// --------------------End Set primitive topology to triangle list (groups of 3 vertices)------------------------------------------------


	// -------------------------Viewport Stage----------------------------------
	D3D11_VIEWPORT vp = {
		// X position of the left hand side of the viewport
		.TopLeftX = 0,
		// Y position of the top of the viewport
		.TopLeftY = 0,
		// Width of the viewport
		.Width = 800,
		// Height of the viewport
		.Height = 600,
		// Minimum depth of the viewport
		.MinDepth = 0,
		// Maximum depth of the viewport
		.MaxDepth = 1,
	};
	// Bind an array of viewports to the rasterizer stage of the pipeline
	pContext_->RSSetViewports(
		// Number of viewports to bind
		1u,
		// An array of D3D11_VIEWPORT structures to bind to the device
		&vp);
	// ----------------------End Viewport Stage----------------------------------


	// we only need to wrap Draw calls with GFX_THROW_INFO_ONLY. Others won't provide useful information
	GFX_THROW_INFO_ONLY(pContext_->DrawIndexed((UINT)std::size(indices), 0u, 0u));
}

void Graphics::Draw16_Cube(float angle, float x, float z)
{
	HRESULT hr;

	// ----------------------------Buffer Creation Stage-------------------------------------
	struct Vertex
	{
		struct
		{
			float x;
			float y;
			float z;
		} pos;

		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;
	};

	// create vertex buffer 
	Vertex vertices[] =
	{
		{-1.0f, -1.0f, -1.0f, 255, 0, 0},
		{1.0f, -1.0f, -1.0f, 0, 255, 0},
		{-1.0f, 1.0f, -1.0f, 0, 0, 255},
		{1.0f, 1.0f, -1.0f, 255, 255, 0},
		{-1.0f, -1.0f, 1.0f, 255, 0, 255},
		{1.0f, -1.0f, 1.0f, 0, 255, 255},
		{-1.0f, 1.0f, 1.0f, 0, 0, 0},
		{1.0f, 1.0f, 1.0f, 255, 255, 255},
	};
	vertices[0].color.g = 255;

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;

	// Describes a buffer resource
	D3D11_BUFFER_DESC bd = {
		// Size of the buffer in bytes
		.ByteWidth = sizeof(vertices),
		// Identify how the buffer is expected to be read from and written to
		.Usage = D3D11_USAGE_DEFAULT,
		// Identify how the buffer will be bound to the pipeline
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		// CPU access flags
		.CPUAccessFlags = 0u,
		// Miscellaneous flags
		.MiscFlags = 0u,
		// The size of each element in the buffer structure (in bytes) when the buffer represents a structured buffer
		.StructureByteStride = sizeof(Vertex),
	};

	// Specifies data for initializing a subresource
	D3D11_SUBRESOURCE_DATA sd = {
		// Pointer to the initialization data
		.pSysMem = vertices,
		// The distance (in bytes) from the beginning of one line of a texture to the next line
		.SysMemPitch = 0u,
		// The distance (in bytes) from the beginning of one depth level to the next
		.SysMemSlicePitch = 0u,
	};

	// Creates a buffer (vertex buffer, index buffer, or shader-constant buffer)
	GFX_THROW_INFO(pDevice_->CreateBuffer(
		// A pointer to a D3D11_BUFFER_DESC structure that describes the buffer
		&bd,
		// A pointer to a D3D11_SUBRESOURCE_DATA structure that describes the initialization data
		&sd,
		// Address of a pointer to the ID3D11Buffer interface for the buffer object created.
		&pVertexBuffer));

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;

	// Bind an array of vertex buffers to the input-assembler stage.
	pContext_->IASetVertexBuffers(
		// The first input slot for binding
		0u,
		// The number of vertex buffers in the array
		1u,
		// A pointer to an array of vertex buffers (Pay special attention here: DO NO USE & OPERATOR!)
		pVertexBuffer.GetAddressOf(),
		// Pointer to an array of stride values
		&stride,
		// Pointer to an array of offset values
		&offset
	);

	// create index buffer
	const unsigned short indices[] =
	{
		0, 2, 1, 2, 3, 1,
		1, 3, 5, 3, 7, 5,
		2, 6, 3, 3, 6, 7,
		4, 5, 7, 4, 7, 6,
		0, 4, 2, 2, 4, 6,
		0, 1, 4, 1, 5, 4
	};

	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ibd = {
		.ByteWidth = sizeof(indices),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags = 0u,
		.MiscFlags = 0u,
		.StructureByteStride = sizeof(unsigned short),
	};

	D3D11_SUBRESOURCE_DATA isd = {
		.pSysMem = indices,
	};

	GFX_THROW_INFO(pDevice_->CreateBuffer(&ibd, &isd, &pIndexBuffer));

	// bind index buffer
	pContext_->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

	// create constant buffer for transformation matrix

	struct ConstantBuffer
	{
		dx::XMMATRIX transform;
	};
	const ConstantBuffer cb =
	{
		{
			dx::XMMatrixTranspose(
				dx::XMMatrixRotationZ(angle) *
				dx::XMMatrixRotationX(angle) *
				// move the cube inside the depth range ([0.5, 10] in this case)
				dx::XMMatrixTranslation(x, z, 4.0f) *
				// LH: left hand
				dx::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f)
			)
		}
	};

	wrl::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd = {
		.ByteWidth = sizeof(cb),
		// dynamic b/c we update constant buffer every frame
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		// CPU will write to the buffer every frame
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0u,
		// no need to provide a stride for this constant buffer
		.StructureByteStride = 0u,
	};
	D3D11_SUBRESOURCE_DATA csd = {
		.pSysMem = &cb,
	};
	GFX_THROW_INFO(pDevice_->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	// bind constant buffer to vertex shader
	pContext_->VSSetConstantBuffers(
		0u,
		1u,
		pConstantBuffer.GetAddressOf());

	// ----------------------------End Buffer Creation Stage-------------------------------------


	// ----------------------------Create Shader Stage-------------------------------------
	// create pixel shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	wrl::ComPtr<ID3DBlob> pBlob;
	GFX_THROW_INFO(D3DReadFileToBlob(
		L"Shaders/cso/PixelShader15.cso",
		&pBlob
	));
	GFX_THROW_INFO(pDevice_->CreatePixelShader(
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		nullptr,
		&pPixelShader));

	// bind pixel shader
	pContext_->PSSetShader(
		pPixelShader.Get(),
		nullptr,
		0);

	// create vertex shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_INFO(D3DReadFileToBlob(
		// A pointer to a constant null-terminated (wide) string that contains the name of the file to read into memory
		L"Shaders/cso/VertexShader15.cso",
		// output handle for the blob
		&pBlob
	));
	GFX_THROW_INFO(pDevice_->CreateVertexShader(
		// A pointer to the compiled shader
		pBlob->GetBufferPointer(),
		// Size of the compiled vertex shader
		pBlob->GetBufferSize(),
		// A pointer to a class linkage interface
		nullptr,
		// Address of a pointer to a ID3D11VertexShader interface
		&pVertexShader));

	// bind vertex shader
	pContext_->VSSetShader(
		// Pointer to a vertex shader
		pVertexShader.Get(),
		nullptr,
		0);
	// -------------------------End Create Shader Stage-------------------------------------


	// ----------------------------input (vertex) layout (2D position only)---------------------------------
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{
			// The HLSL semantic associated with this element in a shader input-signature
			"Position",
			// The semantic index for the element
			0,
			// The data type of the element data
			DXGI_FORMAT_R32G32B32_FLOAT,
			// An integer value that identifies the input-assemble
			0,
			// Offset (in bytes) from the start of the vertex
			0,
			// Identifies the input data class for a single input slot
			D3D11_INPUT_PER_VERTEX_DATA,
			// The number of instances to draw using the same per-instance data before advancing in the buffer by one element
			0
		},
		{
			// The HLSL semantic associated with this element in a shader input-signature
			"Color",
			// The semantic index for the element
			0,
			// The data type of the element data (need to use UNORM)
			DXGI_FORMAT_R8G8B8A8_UNORM,
			// An integer value that identifies the input-assemble
			0,
			// Offset (in bytes) from the start of the vertex
			12u,
			// Identifies the input data class for a single input slot
			D3D11_INPUT_PER_VERTEX_DATA,
			// The number of instances to draw using the same per-instance data before advancing in the buffer by one element
			0
		}
	};
	GFX_THROW_INFO(pDevice_->CreateInputLayout(
		// An array of the input-assembler stage input data types
		ied,
		// The number of input-data types in the array of input-elements
		(UINT)std::size(ied),
		// A pointer to the compiled (vertex) shader
		pBlob->GetBufferPointer(),
		// Size of the compiled (vertex) shader
		pBlob->GetBufferSize(),
		&pInputLayout));
	// bind input layout
	pContext_->IASetInputLayout(pInputLayout.Get());
	// -------------------------End input (vertex) layout (2D position only)---------------------------------


	// -------------------------Bind Render Target(how would D3D know where you want to render to otherwise?)---------------------------
	pContext_->OMSetRenderTargets(
		// Number of render targets to bind
		1u,
		// Pointer to an array of ID3D11RenderTargetView that represent the render targets to bind to the device
		pTarget_.GetAddressOf(),
		// Pointer to a ID3D11DepthStencilView that represents the depth-stencil view to bind to the device
		pDSV_.Get());
	// ----------------------End Bind Render Target------------------------------------------------


	// --------------------Set primitive topology to triangle list (groups of 3 vertices)------------------------------------------------
	pContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// --------------------End Set primitive topology to triangle list (groups of 3 vertices)------------------------------------------------


	// -------------------------Viewport Stage----------------------------------
	D3D11_VIEWPORT vp = {
		// X position of the left hand side of the viewport
		.TopLeftX = 0,
		// Y position of the top of the viewport
		.TopLeftY = 0,
		// Width of the viewport
		.Width = 800,
		// Height of the viewport
		.Height = 600,
		// Minimum depth of the viewport
		.MinDepth = 0,
		// Maximum depth of the viewport
		.MaxDepth = 1,
	};
	// Bind an array of viewports to the rasterizer stage of the pipeline
	pContext_->RSSetViewports(
		// Number of viewports to bind
		1u,
		// An array of D3D11_VIEWPORT structures to bind to the device
		&vp);
	// ----------------------End Viewport Stage----------------------------------


	// we only need to wrap Draw calls with GFX_THROW_INFO_ONLY. Others won't provide useful information
	GFX_THROW_INFO_ONLY(pContext_->DrawIndexed((UINT)std::size(indices), 0u, 0u));
}

void Graphics::Draw16_Cube_2(float angle, float x, float z)
{
	HRESULT hr;

	// ----------------------------Buffer Creation Stage-------------------------------------
	struct Vertex
	{
		struct
		{
			float x;
			float y;
			float z;
		} pos;
	};

	// create vertex buffer 
	Vertex vertices[] =
	{
		{-1.0f, -1.0f, -1.0f},
		{1.0f, -1.0f, -1.0f},
		{-1.0f, 1.0f, -1.0f},
		{1.0f, 1.0f, -1.0f},
		{-1.0f, -1.0f, 1.0f},
		{1.0f, -1.0f, 1.0f},
		{-1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f},
	};

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;

	// Describes a buffer resource
	D3D11_BUFFER_DESC bd = {
		// Size of the buffer in bytes
		.ByteWidth = sizeof(vertices),
		// Identify how the buffer is expected to be read from and written to
		.Usage = D3D11_USAGE_DEFAULT,
		// Identify how the buffer will be bound to the pipeline
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		// CPU access flags
		.CPUAccessFlags = 0u,
		// Miscellaneous flags
		.MiscFlags = 0u,
		// The size of each element in the buffer structure (in bytes) when the buffer represents a structured buffer
		.StructureByteStride = sizeof(Vertex),
	};

	// Specifies data for initializing a subresource
	D3D11_SUBRESOURCE_DATA sd = {
		// Pointer to the initialization data
		.pSysMem = vertices,
		// The distance (in bytes) from the beginning of one line of a texture to the next line
		.SysMemPitch = 0u,
		// The distance (in bytes) from the beginning of one depth level to the next
		.SysMemSlicePitch = 0u,
	};

	// Creates a buffer (vertex buffer, index buffer, or shader-constant buffer)
	GFX_THROW_INFO(pDevice_->CreateBuffer(
		// A pointer to a D3D11_BUFFER_DESC structure that describes the buffer
		&bd,
		// A pointer to a D3D11_SUBRESOURCE_DATA structure that describes the initialization data
		&sd,
		// Address of a pointer to the ID3D11Buffer interface for the buffer object created.
		&pVertexBuffer));

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;

	// Bind an array of vertex buffers to the input-assembler stage.
	pContext_->IASetVertexBuffers(
		// The first input slot for binding
		0u,
		// The number of vertex buffers in the array
		1u,
		// A pointer to an array of vertex buffers (Pay special attention here: DO NO USE & OPERATOR!)
		pVertexBuffer.GetAddressOf(),
		// Pointer to an array of stride values
		&stride,
		// Pointer to an array of offset values
		&offset
	);

	// create index buffer
	const unsigned short indices[] =
	{
		0, 2, 1, 2, 3, 1,
		1, 3, 5, 3, 7, 5,
		2, 6, 3, 3, 6, 7,
		4, 5, 7, 4, 7, 6,
		0, 4, 2, 2, 4, 6,
		0, 1, 4, 1, 5, 4
	};

	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ibd = {
		.ByteWidth = sizeof(indices),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags = 0u,
		.MiscFlags = 0u,
		.StructureByteStride = sizeof(unsigned short),
	};

	D3D11_SUBRESOURCE_DATA isd = {
		.pSysMem = indices,
	};

	GFX_THROW_INFO(pDevice_->CreateBuffer(&ibd, &isd, &pIndexBuffer));

	// bind index buffer
	pContext_->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

	// create constant buffer for transformation matrix

	struct ConstantBuffer
	{
		dx::XMMATRIX transform;
	};
	const ConstantBuffer cb =
	{
		{
			dx::XMMatrixTranspose(
				dx::XMMatrixRotationZ(angle) *
				dx::XMMatrixRotationX(angle) *
				// move the cube inside the depth range ([0.5, 10] in this case)
				dx::XMMatrixTranslation(x, 0.0f, z + 4.0f) *
				// LH: left hand
				dx::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f)
			)
		}
	};

	wrl::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd = {
		.ByteWidth = sizeof(cb),
		// dynamic b/c we update constant buffer every frame
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		// CPU will write to the buffer every frame
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0u,
		// no need to provide a stride for this constant buffer
		.StructureByteStride = 0u,
	};
	D3D11_SUBRESOURCE_DATA csd = {
		.pSysMem = &cb,
	};
	GFX_THROW_INFO(pDevice_->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	// bind constant buffer to vertex shader
	pContext_->VSSetConstantBuffers(
		0u,
		1u,
		pConstantBuffer.GetAddressOf());

	// create constant buffer for lookup table for cube face colors used in pixel shader

	struct ConstantBuffer2
	{
		struct
		{
			float r;
			float g;
			float b;
			// for proper alignment, we include a
			float a;
		} face_colors[6];
	};
	const ConstantBuffer2 cb2 =
	{
		{
			// for the first two triangles
			{1.0f, 0.0f, 1.0f},
			// for the second triangles..
			{1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f},
			{1.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 1.0f},
		}
	};
	wrl::ComPtr<ID3D11Buffer> pConstantBuffer2;
	D3D11_BUFFER_DESC cbd2 = {
		.ByteWidth = sizeof(cb2),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = 0u,
		.MiscFlags = 0u,
		.StructureByteStride = 0u,
	};
	D3D11_SUBRESOURCE_DATA csd2 = {
		.pSysMem = &cb2,
	};
	GFX_THROW_INFO(pDevice_->CreateBuffer(&cbd2, &csd2, &pConstantBuffer2));

	// bind constant buffer to pixel shader
	pContext_->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf());

	// ----------------------------End Buffer Creation Stage-------------------------------------


	// ----------------------------Create Shader Stage-------------------------------------
	// create pixel shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	wrl::ComPtr<ID3DBlob> pBlob;
	GFX_THROW_INFO(D3DReadFileToBlob(
		L"Shaders/cso/PixelShader15_2.cso",
		&pBlob
	));
	GFX_THROW_INFO(pDevice_->CreatePixelShader(
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		nullptr,
		&pPixelShader));

	// bind pixel shader
	pContext_->PSSetShader(
		pPixelShader.Get(),
		nullptr,
		0);

	// create vertex shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_INFO(D3DReadFileToBlob(
		// A pointer to a constant null-terminated (wide) string that contains the name of the file to read into memory
		L"Shaders/cso/VertexShader15_2.cso",
		// output handle for the blob
		&pBlob
	));
	GFX_THROW_INFO(pDevice_->CreateVertexShader(
		// A pointer to the compiled shader
		pBlob->GetBufferPointer(),
		// Size of the compiled vertex shader
		pBlob->GetBufferSize(),
		// A pointer to a class linkage interface
		nullptr,
		// Address of a pointer to a ID3D11VertexShader interface
		&pVertexShader));

	// bind vertex shader
	pContext_->VSSetShader(
		// Pointer to a vertex shader
		pVertexShader.Get(),
		nullptr,
		0);
	// -------------------------End Create Shader Stage-------------------------------------


	// ----------------------------input (vertex) layout (2D position only)---------------------------------
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{
			// The HLSL semantic associated with this element in a shader input-signature
			"Position",
			// The semantic index for the element
			0,
			// The data type of the element data
			DXGI_FORMAT_R32G32B32_FLOAT,
			// An integer value that identifies the input-assemble
			0,
			// Offset (in bytes) from the start of the vertex
			0,
			// Identifies the input data class for a single input slot
			D3D11_INPUT_PER_VERTEX_DATA,
			// The number of instances to draw using the same per-instance data before advancing in the buffer by one element
			0
		}
	};
	GFX_THROW_INFO(pDevice_->CreateInputLayout(
		// An array of the input-assembler stage input data types
		ied,
		// The number of input-data types in the array of input-elements
		(UINT)std::size(ied),
		// A pointer to the compiled (vertex) shader
		pBlob->GetBufferPointer(),
		// Size of the compiled (vertex) shader
		pBlob->GetBufferSize(),
		&pInputLayout));
	// bind input layout
	pContext_->IASetInputLayout(pInputLayout.Get());
	// -------------------------End input (vertex) layout (2D position only)---------------------------------


	// -------------------------Bind Render Target(how would D3D know where you want to render to otherwise?)---------------------------
	pContext_->OMSetRenderTargets(
		// Number of render targets to bind
		1u,
		// Pointer to an array of ID3D11RenderTargetView that represent the render targets to bind to the device
		pTarget_.GetAddressOf(),
		// Pointer to a ID3D11DepthStencilView that represents the depth-stencil view to bind to the device
		pDSV_.Get());
	// ----------------------End Bind Render Target------------------------------------------------


	// --------------------Set primitive topology to triangle list (groups of 3 vertices)------------------------------------------------
	pContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// --------------------End Set primitive topology to triangle list (groups of 3 vertices)------------------------------------------------


	// -------------------------Viewport Stage----------------------------------
	D3D11_VIEWPORT vp = {
		// X position of the left hand side of the viewport
		.TopLeftX = 0,
		// Y position of the top of the viewport
		.TopLeftY = 0,
		// Width of the viewport
		.Width = 800,
		// Height of the viewport
		.Height = 600,
		// Minimum depth of the viewport
		.MinDepth = 0,
		// Maximum depth of the viewport
		.MaxDepth = 1,
	};
	// Bind an array of viewports to the rasterizer stage of the pipeline
	pContext_->RSSetViewports(
		// Number of viewports to bind
		1u,
		// An array of D3D11_VIEWPORT structures to bind to the device
		&vp);
	// ----------------------End Viewport Stage----------------------------------


	// we only need to wrap Draw calls with GFX_THROW_INFO_ONLY. Others won't provide useful information
	GFX_THROW_INFO_ONLY(pContext_->DrawIndexed((UINT)std::size(indices), 0u, 0u));
}*/

// -------------------------------------------------------------------------

Graphics::HrException::HrException(int line,
                                   const char* file,
                                   HRESULT hr,
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

const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "3D Game Programming Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

// -----------------------------------------------------------------
Graphics::InfoException::InfoException(int line,
                                       const char* file,
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
	return "Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept
{
	return info_;
}
