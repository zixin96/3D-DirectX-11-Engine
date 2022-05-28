#include "TransformCbuf.h"

namespace D3DEngine
{
	TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent, UINT slot)
		:
		parent_(parent)
	{
		if (!pVertexCbuf_)
		{
			pVertexCbuf_ = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, slot);
		}
	}

	void TransformCbuf::Bind(Graphics& gfx) noexcept
	{
		UpdateBindImpl(gfx, GetTransforms(gfx));
	}

	void TransformCbuf::UpdateBindImpl(Graphics& gfx, const Transforms& tf) noexcept
	{
		pVertexCbuf_->Update(gfx, tf);
		pVertexCbuf_->Bind(gfx);
	}

	TransformCbuf::Transforms TransformCbuf::GetTransforms(Graphics& gfx) noexcept
	{
		const auto modelView = parent_.GetTransformXM() * gfx.GetCamera();
		return {
			DirectX::XMMatrixTranspose(
			                           modelView *
			                           gfx.GetProjection()
			                          ),
			DirectX::XMMatrixTranspose(modelView)
		};
	}

	std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::pVertexCbuf_;
}
