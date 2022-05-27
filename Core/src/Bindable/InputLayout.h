#pragma once
#include "Bindable.h"
#include "Drawable/Complex/VertexView.h"

namespace D3DEngine
{
	class InputLayout : public Bindable
	{
		public:
			InputLayout(Graphics& gfx, DynamicVertexLayout layout, ID3DBlob* pVertexShaderBytecode);
			void                             Bind(Graphics& gfx) noexcept override;
			static std::shared_ptr<InputLayout> Resolve(Graphics& gfx, const DynamicVertexLayout& layout, ID3DBlob* pVertexShaderBytecode);
			static std::string               GenerateUID(const DynamicVertexLayout& layout, ID3DBlob* pVertexShaderBytecode = nullptr);
			std::string                      GetUID() const noexcept override;
		protected:
			DynamicVertexLayout                       layout_;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout_;
	};
}
