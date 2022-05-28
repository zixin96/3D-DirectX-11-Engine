#include "TransformCbufDouble.h"

namespace D3DEngine
{
	TransformCbufDouble::TransformCbufDouble(Graphics& gfx, const Drawable& parent, UINT slotV, UINT slotP)
		: TransformCbuf(gfx, parent, slotV)
	{
		if (!pPcbuf_)
		{
			pPcbuf_ = std::make_unique<PixelConstantBuffer<Transforms>>(gfx, slotP);
		}
	}

	void TransformCbufDouble::Bind(Graphics& gfx) noexcept
	{
		const auto tf = GetTransforms(gfx);
		TransformCbuf::UpdateBindImpl(gfx, tf);
		UpdateBindImpl(gfx, tf);
	}

	void TransformCbufDouble::UpdateBindImpl(Graphics& gfx, const Transforms& tf) noexcept
	{
		pPcbuf_->Update(gfx, tf);
		pPcbuf_->Bind(gfx);
	}

	std::unique_ptr<PixelConstantBuffer<TransformCbuf::Transforms>> TransformCbufDouble::pPcbuf_;
}
