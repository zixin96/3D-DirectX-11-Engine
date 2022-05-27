#pragma once
#include "ConstantBuffers.h"
#include "Drawable/Drawable.h"

namespace D3DEngine
{
	/**
	 * \brief TransformCbuf acts as a wrapper around Vertex Constant buffer.
	 * For our current vertex shader, we have one constant buffer that holds the transformation matrices that is applied to each vertex.
	 * Since we need to update vertex constant buffer every frame, we create this class to update and bind vertex constant buffer for us.
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
			// Each drawable has a single vertex constant buffer that is shared among all instances. 
			// We can overwrite it with the data of the new instance when necessary
			// Why use unique pointer? so that we can lazy-allocated later in the constructor
			static std::unique_ptr<VertexConstantBuffer<Transforms>> pVertexCbuf_;
			// we need the drawable object b/c transformation matrices are built on top of the object's transformation (this is distinct per instance)
			const Drawable& parent_;
	};
}
