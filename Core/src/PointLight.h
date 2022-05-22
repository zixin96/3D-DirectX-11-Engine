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
		// pixel shader expects float3 to have 16-bytes alignment
		// so we need to explicitly tell compiler to add paddings 
		alignas(16) DirectX::XMFLOAT3 pos;
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 diffuseColor;
		float diffuseIntensity;
		float attConst;
		float attLin;
		float attQuad;
	};

	PointLightCBuf cbData_;
	// visual representation of the light in the world
	mutable SolidSphere mesh_;
	mutable PixelConstantBuffer<PointLightCBuf> cbuf_;
};
