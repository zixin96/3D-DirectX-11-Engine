#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent, UINT slot)
	:
	parent_(parent)
{
	if (!pVertexCbuf_)
	{
		pVertexCbuf_ = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, slot);
	}
}

// update and bind vertex constant buffer
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

	pVertexCbuf_->Update(gfx, tf);
	pVertexCbuf_->Bind(gfx);
}

std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::pVertexCbuf_;
