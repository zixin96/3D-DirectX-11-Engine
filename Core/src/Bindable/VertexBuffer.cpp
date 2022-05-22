#include "VertexBuffer.h"

void VertexBuffer::Bind(Graphics& gfx) noexcept
{
	const UINT offset = 0u;
	// Bind an array of vertex buffers to the input-assembler stage.
	GetContext(gfx)->IASetVertexBuffers(
		// The first input slot for binding
		0u,
		// The number of vertex buffers in the array
		1u,
		// A pointer to an array of vertex buffers (Pay special attention here: DO NO USE & OPERATOR!)
		pVertexBuffer_.GetAddressOf(),
		// Pointer to an array of stride values
		&stride_,
		// Pointer to an array of offset values
		&offset
	);
}
