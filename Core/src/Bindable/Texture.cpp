#include "Texture.h"
#include "Utils/Surface.h"
#include "Debug/GraphicsThrowMacros.h"

namespace wrl = Microsoft::WRL;

Texture::Texture(Graphics& gfx, const Surface& s)
{
	INFOMAN(gfx);

	// create texture resource

	// Describes a 2D texture
	D3D11_TEXTURE2D_DESC textureDesc = {
		.Width = s.GetWidth(),
		.Height = s.GetHeight(),
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
		// no AA
		.SampleDesc = {.Count = 1, .Quality = 0},
		.Usage = D3D11_USAGE_DEFAULT,
		// bind it as resource to the pipeline used by shaders
		.BindFlags = D3D11_BIND_SHADER_RESOURCE,
		// CPU won't touch it
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
	};

	// initialize with data
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = s.GetBufferPtr();
	// distance in bytes between the first pixel in row 0 and the first pixel in row 1 
	sd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color);

	wrl::ComPtr<ID3D11Texture2D> pTexture;
	GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
		&textureDesc,
		&sd,
		&pTexture
	));

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
		pTexture.Get(), &srvDesc, &pTextureView_
	));
}

void Texture::Bind(Graphics& gfx) noexcept
{
	// textures are used by pixel shaders
	GetContext(gfx)->PSSetShaderResources(0u, 1u, pTextureView_.GetAddressOf());
}
