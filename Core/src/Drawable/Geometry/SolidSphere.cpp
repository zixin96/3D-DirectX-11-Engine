#include "SolidSphere.h"
#include "Bindable/BindableCommon.h"
#include "Debug/GraphicsThrowMacros.h"
#include "Sphere.h"
#include "Drawable/Complex/VertexView.h"

namespace D3DEngine
{
	SolidSphere::SolidSphere(Graphics& gfx, float radius)
	{
		namespace dx = DirectX;
		auto model = Sphere::Make();
		model.Transform(dx::XMMatrixScaling(radius, radius, radius));
		const auto geometryTag = "$sphere." + std::to_string(radius);
		AddBind(VertexBuffer::Resolve(gfx, geometryTag, model.vertices));
		AddBind(IndexBuffer::Resolve(gfx, geometryTag, model.indices));

		auto pvs   = VertexShader::Resolve(gfx, "Shaders/cso/SolidVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddBind(std::move(pvs));

		AddBind(PixelShader::Resolve(gfx, "Shaders/cso/SolidPS.cso"));

		struct PSColorConstant
		{
			dx::XMFLOAT3 color = {1.0f, 1.0f, 1.0f};
			float        padding;
		}                colorConst;

		AddBind(PixelConstantBuffer<PSColorConstant>::Resolve(gfx, colorConst, 2));

		AddBind(InputLayout::Resolve(gfx, model.vertices.GetLayout(), pvsbc));

		AddBind(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		// TODO: Why is this not resolve()?
		AddBind(std::make_shared<TransformCbuf>(gfx, *this));
	}

	void SolidSphere::SetPos(DirectX::XMFLOAT3 pos) noexcept
	{
		this->pos = pos;
	}

	DirectX::XMMATRIX SolidSphere::GetTransformXM() const noexcept
	{
		return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	}
}
