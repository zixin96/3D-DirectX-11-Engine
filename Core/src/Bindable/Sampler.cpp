#include "Sampler.h"
#include "Debug/GraphicsThrowMacros.h"

Sampler::Sampler(Graphics& gfx)
{
	INFOMAN(gfx);

	D3D11_SAMPLER_DESC samplerDesc = {
		.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
		.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
		.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
		.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
	};

	GFX_THROW_INFO(GetDevice( gfx )->CreateSamplerState( &samplerDesc,&pSampler_ ));
}

void Sampler::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->PSSetSamplers(0, 1, pSampler_.GetAddressOf());
}
