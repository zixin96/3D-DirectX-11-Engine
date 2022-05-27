#include "VertexBuffer.h"
#include "BindableCodex.h"

namespace D3DEngine
{
	// construct a vertex buffer without interacting with our central repo system 
	VertexBuffer::VertexBuffer(Graphics& gfx, const RawVertexBufferWithLayout& vbuf)
	// "?" means that this bindable is not in the repo system  (you can mark any bindable with tag ? to indicate that this bindable is separate from our central repo system) 
		: VertexBuffer(gfx, "?", vbuf)
	{
	}

	VertexBuffer::VertexBuffer(Graphics& gfx, const std::string& tag, const RawVertexBufferWithLayout& vbuf)
		:
		stride_((UINT)vbuf.GetLayout().Size()),
		tag_(tag)
	{
		INFOMAN(gfx);

		D3D11_BUFFER_DESC bd      = {};
		bd.BindFlags              = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage                  = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags         = 0u;
		bd.MiscFlags              = 0u;
		bd.ByteWidth              = UINT(vbuf.SizeBytes());
		bd.StructureByteStride    = stride_;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem                = vbuf.GetData();
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &sd, &pVertexBuffer_));
	}

	void VertexBuffer::Bind(Graphics& gfx) noexcept
	{
		const UINT offset = 0u;
		GetContext(gfx)->IASetVertexBuffers(0u, 1u, pVertexBuffer_.GetAddressOf(), &stride_, &offset);
	}

	std::shared_ptr<VertexBuffer> VertexBuffer::Resolve(Graphics&                        gfx,
	                                                    const std::string&               tag,
	                                                    const RawVertexBufferWithLayout& vbuf)
	{
		assert(tag != "?");
		return Codex::Resolve<VertexBuffer>(gfx, tag, vbuf);
	}

	std::string VertexBuffer::GenerateUID_(const std::string& tag)
	{
		using namespace std::string_literals;
		return typeid(VertexBuffer).name() + "#"s + tag;
	}

	std::string VertexBuffer::GetUID() const noexcept
	{
		return GenerateUID(tag_);
	}
}
