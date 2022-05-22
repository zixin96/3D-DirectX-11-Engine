#pragma once
#include "Bindable.h"

/**
 * \brief Specifies how you perform look-up in textures
 */
class Sampler : public Bindable
{
public:
	Sampler(Graphics& gfx);
	void Bind(Graphics& gfx) noexcept override;
protected:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler_;
};
