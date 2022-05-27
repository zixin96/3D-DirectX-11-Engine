#include "IndexBuffer.h"

#include "BindableCodex.h"
#include "Debug/GraphicsThrowMacros.h"

namespace D3DEngine
{
	// construct an index buffer without interacting with our central repo system
	IndexBuffer::IndexBuffer(Graphics& gfx, const std::vector<unsigned short>& indices)
	// "?" means that this bindable is not in the repo system  (you can mark any bindable with tag ? to indicate that this bindable is separate from our central repo system) 
		: IndexBuffer(gfx, "?", indices)
	{
	}

	IndexBuffer::IndexBuffer(Graphics& gfx, const std::string& tag, const std::vector<unsigned short>& indices)
		:
		tag_(tag),
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

		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&ibd, &isd, &pIndexBuffer_));
	}

	void IndexBuffer::Bind(Graphics& gfx) noexcept
	{
		GetContext(gfx)->IASetIndexBuffer(pIndexBuffer_.Get(), DXGI_FORMAT_R16_UINT, 0u);
	}

	UINT IndexBuffer::GetCount() const noexcept
	{
		return count_;
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::Resolve(Graphics&                          gfx,
	                                                  const std::string&                 tag,
	                                                  const std::vector<unsigned short>& indices)
	{
		assert(tag != "?");
		return Codex::Resolve<IndexBuffer>(gfx, tag, indices);
	}

	std::string IndexBuffer::GenerateUID_(const std::string& tag)
	{
		using namespace std::string_literals;
		return typeid(IndexBuffer).name() + "#"s + tag;
	}

	std::string IndexBuffer::GetUID() const noexcept
	{
		return GenerateUID_(tag_);
	}
}
