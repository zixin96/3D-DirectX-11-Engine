#pragma once
#include "Graphics.h"

namespace D3DEngine
{
	class Bindable;
	class IndexBuffer;

	class Drawable
	{
		public:
			Drawable()                = default;
			virtual ~Drawable()       = default;
			Drawable(const Drawable&) = delete;
			virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
			void                      Draw(Graphics& gfx) const noxnd;
		protected:
			// query instance of bindable to be changed
			template <class T>
			T* QueryBindable() noexcept
			{
				for (auto& pb : binds_)
				{
					if (auto pt = dynamic_cast<T*>(pb.get()))
					{
						return pt;
					}
				}
				return nullptr;
			}

			void AddBind(std::shared_ptr<Bindable> bind) noxnd;
		private:
			const IndexBuffer*                     pIndexBuffer_ = nullptr;
			std::vector<std::shared_ptr<Bindable>> binds_;
	};
}
