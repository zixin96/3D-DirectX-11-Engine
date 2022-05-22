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
	const auto model = parent_.GetTransformXM();
	const Transforms tf =
	{
		.modelViewProj = DirectX::XMMatrixTranspose(
			model *
			gfx.GetCamera() *
			gfx.GetProjection()
		),
		.model = DirectX::XMMatrixTranspose(model),
	};

	pVcbuf_->Update(gfx, tf);
	pVcbuf_->Bind(gfx);
}

std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::pVcbuf_;
