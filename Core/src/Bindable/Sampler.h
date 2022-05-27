#pragma once
#include "Bindable.h"

namespace D3DEngine
{
	/**
	 * \brief Specifies how you perform look-up in textures
	 */
	class Sampler : public Bindable
	{
		public:
			Sampler(Graphics& gfx);
			void                             Bind(Graphics& gfx) noexcept override;
			static std::shared_ptr<Sampler> Resolve(Graphics& gfx);
			static std::string               GenerateUID();
			std::string                      GetUID() const noexcept override;
		protected:
			Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler_;
	};
}
