#pragma once
#include "Graphics.h"
#include "Drawable/SolidSphere.h"
#include "Bindable/ConstantBuffers.h"

/**
 * \brief Each scene has a set of point light globally
 */
class PointLight
{
public:
	PointLight(Graphics& gfx, float radius = 0.5f);
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;
	void Draw(Graphics& gfx) const noexcept(!IS_DEBUG);
	void Bind(Graphics& gfx) const noexcept;
private:
	struct PointLightCBuf
	{
		DirectX::XMFLOAT3 pos;
		float padding;
	};

	DirectX::XMFLOAT3 pos_ = {0.0f, 0.0f, 0.0f};
	// visual representation of the light in the world
	mutable SolidSphere mesh_;
	mutable PixelConstantBuffer<PointLightCBuf> cbuf_;
};
