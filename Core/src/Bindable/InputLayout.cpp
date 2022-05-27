#include "InputLayout.h"

#include "BindableCodex.h"
#include "Debug/GraphicsThrowMacros.h"

namespace D3DEngine
{
	InputLayout::InputLayout(Graphics&           gfx,
	                         DynamicVertexLayout layout_in,
	                         ID3DBlob*           pVertexShaderBytecode)
		: layout_(std::move(layout_in))
	{
		INFOMAN(gfx);

		const auto d3dLayout = layout_.GetD3DLayout();

		GFX_THROW_INFO(GetDevice(gfx)->CreateInputLayout(
			               d3dLayout.data(),
			               (UINT)d3dLayout.size(),
			               pVertexShaderBytecode->GetBufferPointer(),
			               pVertexShaderBytecode->GetBufferSize(),
			               &pInputLayout_
		               ));
	}

	void InputLayout::Bind(Graphics& gfx) noexcept
	{
		GetContext(gfx)->IASetInputLayout(pInputLayout_.Get());
	}

	std::shared_ptr<InputLayout> InputLayout::Resolve(Graphics&                  gfx,
	                                               const DynamicVertexLayout& layout,
	                                               ID3DBlob*                  pVertexShaderBytecode)
	{
		return Codex::Resolve<InputLayout>(gfx, layout, pVertexShaderBytecode);
	}

	std::string InputLayout::GenerateUID(const DynamicVertexLayout& layout, ID3DBlob* pVertexShaderBytecode)
	{
		using namespace std::string_literals;
		return typeid(InputLayout).name() + "#"s + layout.GetCode();
	}

	std::string InputLayout::GetUID() const noexcept
	{
		return GenerateUID(layout_);
	}
}
