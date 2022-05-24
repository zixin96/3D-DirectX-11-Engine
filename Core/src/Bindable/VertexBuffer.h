#pragma once
#include "Bindable.h"
#include "Vertex.h"
#include "Debug/GraphicsThrowMacros.h"

class VertexBuffer : public Bindable
{
public:
	// TODO: Why make this a template? 
	template <class V>
	VertexBuffer(Graphics& gfx, const std::vector<V>& vertices)
		:
		stride_(sizeof(V))
	{
		INFOMAN(gfx);

		// Describes vertex buffer resource
		D3D11_BUFFER_DESC bd = {
			// Size of the buffer in bytes
			.ByteWidth = UINT(sizeof(V) * vertices.size()),
			// Identify how the buffer is expected to be read from and written to
			.Usage = D3D11_USAGE_DEFAULT,
			// Identify how the buffer will be bound to the pipeline
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			// CPU access flags
			.CPUAccessFlags = 0u,
			// Miscellaneous flags
			.MiscFlags = 0u,
			// The size of each element in the buffer structure (in bytes) when the buffer represents a structured buffer
			.StructureByteStride = sizeof(V),
		};

		// Specifies data for initializing a subresource
		D3D11_SUBRESOURCE_DATA sd = {
			// Pointer to the initialization data
			.pSysMem = vertices.data(),
			// The distance (in bytes) from the beginning of one line of a texture to the next line
			.SysMemPitch = 0u,
			// The distance (in bytes) from the beginning of one depth level to the next
			.SysMemSlicePitch = 0u,
		};

		// Creates vertex buffer
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(
			// A pointer to a D3D11_BUFFER_DESC structure that describes the buffer
			&bd,
			// A pointer to a D3D11_SUBRESOURCE_DATA structure that describes the initialization data
			&sd,
			// Address of a pointer to the ID3D11Buffer interface for the buffer object created.
			&pVertexBuffer_));
	}

	VertexBuffer(Graphics& gfx, const D3DEngine::VertexBuffer& vbuf)
		:
		stride_((UINT)vbuf.GetLayout().Size())
	{
		INFOMAN(gfx);

		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(vbuf.SizeBytes());
		bd.StructureByteStride = stride_;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vbuf.GetData();
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &sd, &pVertexBuffer_));
	}

	void Bind(Graphics& gfx) noexcept override;
protected:
	UINT stride_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer_;
};
