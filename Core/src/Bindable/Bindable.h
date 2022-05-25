#pragma once
#include "Graphics.h"


/**
 * \brief An interface presenting all bindable objects (like pixel shader, input layout, etc)
 */
class Bindable
{
public:
	virtual void Bind(Graphics& gfx) noexcept = 0;
	virtual ~Bindable() = default;
protected:
	// children of Bindable will have access to Graphics' private member variables through these static functions:
	static ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept;
	static ID3D11Device* GetDevice(Graphics& gfx) noexcept;
	static DxgiInfoManager& GetInfoManager(Graphics& gfx);
};
