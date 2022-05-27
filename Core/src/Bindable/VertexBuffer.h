#pragma once
#include "Bindable.h"
#include "Drawable/Complex/VertexView.h"
#include "Debug/GraphicsThrowMacros.h"

namespace D3DEngine
{
	class VertexBuffer : public Bindable
	{
		public:
			VertexBuffer(Graphics& gfx, const std::string& tag, const RawVertexBufferWithLayout& vbuf);
			VertexBuffer(Graphics& gfx, const RawVertexBufferWithLayout& vbuf);
			void                                 Bind(Graphics& gfx) noexcept override;
			static std::shared_ptr<VertexBuffer> Resolve(Graphics& gfx, const std::string& tag, const RawVertexBufferWithLayout& vbuf);

			template <typename...Ignore>
			static std::string GenerateUID(const std::string& tag, Ignore&&...ignore)
			{
				return GenerateUID_(tag);
			}

			std::string GetUID() const noexcept override;
		private:
			static std::string GenerateUID_(const std::string& tag);
		protected:
			std::string                          tag_; // this tag serves as UID for the vertex buffer
			UINT                                 stride_;
			Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer_;
	};
}
