#pragma once
#include "Bindable.h"

namespace D3DEngine
{
	class Texture : public Bindable
	{
		public:
			Texture(Graphics& gfx, const std::string& path, UINT slot = 0);
			void                             Bind(Graphics& gfx) noexcept override;
			static std::shared_ptr<Texture> Resolve(Graphics& gfx, const std::string& path, UINT slot = 0);
			static std::string               GenerateUID(const std::string& path, UINT slot = 0);
			std::string                      GetUID() const noexcept override;
		private:
			unsigned int slot_;
		protected:
			std::string                                      path_;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView_;
	};
}
