#include "Sampler.h"
#include "BindableCodex.h"
#include "Debug/GraphicsThrowMacros.h"

namespace D3DEngine
{
	Sampler::Sampler(Graphics& gfx)
	{
		INFOMAN(gfx);

		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

		GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &pSampler_));
	}

	void Sampler::Bind(Graphics& gfx) noexcept
	{
		GetContext(gfx)->PSSetSamplers(0, 1, pSampler_.GetAddressOf());
	}

	// call this function if you want to create a sampler
	// we will have a resolve function per bindable
	std::shared_ptr<Sampler> Sampler::Resolve(Graphics& gfx)
	{
		return Codex::Resolve<Sampler>(gfx);
	}

	/**
	 * \brief  Generate a unique ID for this sampler
	 * \return A UID for this sampler
	 */
	std::string Sampler::GenerateUID()
	{
		return typeid(Sampler).name();
	}

	// provide a way to get the UID from an existing bindable (useful if we want to query for specific bindables)
	std::string Sampler::GetUID() const noexcept
	{
		return GenerateUID();
	}
}
