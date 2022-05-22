#include "Bindable.h"

ID3D11DeviceContext* Bindable::GetContext(Graphics& gfx) noexcept
{
	return gfx.pContext_.Get();
}

ID3D11Device* Bindable::GetDevice(Graphics& gfx) noexcept
{
	return gfx.pDevice_.Get();
}

DxgiInfoManager& Bindable::GetInfoManager(Graphics& gfx) noexcept(!IS_DEBUG)
{
#ifndef NDEBUG
	return gfx.infoManager_;
#else
	throw std::logic_error( "YouFuckedUp! (tried to access gfx.infoManager in Release config)" );
#endif
}
