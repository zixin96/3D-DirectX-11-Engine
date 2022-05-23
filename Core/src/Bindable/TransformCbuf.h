#pragma once
#include "ConstantBuffers.h"
#include "Drawable/Drawable.h"
#include <DirectXMath.h>

/**
 * \brief TransformCbuf acts as a wrapper around Vertex Constant buffer.
 * It updates and binds vertex constant buffer for us. 
 */
class TransformCbuf : public Bindable
{
	struct Transforms
	{
		DirectX::XMMATRIX modelViewProj;
		DirectX::XMMATRIX modelView;
	};

public:
	TransformCbuf(Graphics& gfx, const Drawable& parent, UINT slot = 0u);
	void Bind(Graphics& gfx) noexcept override;
private:
	// vertex constant buffer should be shared among all instances of a drawable 
	static std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>> pVcbuf_;
	const Drawable& parent_;
};
