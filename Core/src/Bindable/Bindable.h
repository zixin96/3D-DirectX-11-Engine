#pragma once
#include "Graphics.h"

namespace D3DEngine
{
	/**
	 * \brief An interface presenting all bindable objects (like pixel shader, input layout, etc)
	 */
	class Bindable
	{
		public:
			virtual void Bind(Graphics& gfx) noexcept = 0;

			/**
			 * \brief Return the UID for this bindable
			 * \return Unique ID for this bindable
			 */
			virtual std::string GetUID() const noexcept
			{
				assert(false);
				return "";
			}

			virtual ~Bindable() = default;
		protected:
			// children of Bindable will have access to Graphics' private member variables through these static functions:
			static ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept;
			static ID3D11Device*        GetDevice(Graphics& gfx) noexcept;
			static DxgiInfoManager&     GetInfoManager(Graphics& gfx);
	};
}
