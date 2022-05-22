#include "IndexBuffer.h"
#include "Debug/GraphicsThrowMacros.h"

IndexBuffer::IndexBuffer(Graphics& gfx, const std::vector<unsigned short>& indices)
	:
	count_((UINT)indices.size())
{
	INFOMAN(gfx);

	D3D11_BUFFER_DESC ibd = {
		.ByteWidth = UINT(count_ * sizeof(unsigned short)),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags = 0u,
		.MiscFlags = 0u,
		.StructureByteStride = sizeof(unsigned short),
	};

	D3D11_SUBRESOURCE_DATA isd = {
		.pSysMem = indices.data(),
	};

	GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&ibd,&isd,&pIndexBuffer_));
}

void IndexBuffer::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->IASetIndexBuffer(pIndexBuffer_.Get(), DXGI_FORMAT_R16_UINT, 0u);
}

UINT IndexBuffer::GetCount() const noexcept
{
	return count_;
}
