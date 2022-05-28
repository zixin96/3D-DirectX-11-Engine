#pragma once
#include "ConstantBuffers.h"
#include "TransformCbuf.h"
#include "Drawable/Drawable.h"

namespace D3DEngine
{
	class TransformCbufDouble : public TransformCbuf
	{
		public:
			TransformCbufDouble(Graphics& gfx, const Drawable& parent, UINT slotV = 0u, UINT slotP = 0u);
			void Bind(Graphics& gfx) noexcept override;
		protected:
			void UpdateBindImpl(Graphics& gfx, const Transforms& tf) noexcept;
		private:
			static std::unique_ptr<PixelConstantBuffer<Transforms>> pPcbuf_;
	};
}
