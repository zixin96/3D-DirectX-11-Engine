#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent, UINT slot)
	:
	parent_(parent)
{
	if (!pVcbuf_)
	{
		pVcbuf_ = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, slot);
	}
}

void TransformCbuf::Bind(Graphics& gfx) noexcept
{
	const auto modelView = parent_.GetTransformXM() * gfx.GetCamera();

	const Transforms tf =
	{
		.modelViewProj = DirectX::XMMatrixTranspose(
			modelView *
			gfx.GetProjection()
		),
		.modelView = DirectX::XMMatrixTranspose(
			modelView
		),
	};

	pVcbuf_->Update(gfx, tf);
	pVcbuf_->Bind(gfx);
}

std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::pVcbuf_;
