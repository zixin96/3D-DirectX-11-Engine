#include "PointLight.h"
#include "imgui/imgui.h"

namespace D3DEngine
{
	PointLight::PointLight(Graphics& gfx, float radius)
		:
		mesh_(gfx, radius),
		cbuf_(gfx, 0)
	{
		// initialize lighting parameters when we create the point light
		Reset();
	}

	void PointLight::SpawnControlWindow() noexcept
	{
		if (ImGui::Begin("Light"))
		{
			ImGui::Text("Position");
			ImGui::SliderFloat("X", &cbData_.pos.x, -60.0f, 60.0f, "%.1f");
			ImGui::SliderFloat("Y", &cbData_.pos.y, -60.0f, 60.0f, "%.1f");
			ImGui::SliderFloat("Z", &cbData_.pos.z, -60.0f, 60.0f, "%.1f");

			ImGui::Text("Intensity/Color");
			ImGui::SliderFloat("Intensity", &cbData_.diffuseIntensity, 0.01f, 2.0f, "%.2f");
			ImGui::ColorEdit3("Diffuse Color", &cbData_.diffuseColor.x);
			ImGui::ColorEdit3("Ambient", &cbData_.ambient.x);

			ImGui::Text("Falloff");
			ImGui::SliderFloat("Constant", &cbData_.attConst, 0.05f, 10.0f, "%.2f");
			ImGui::SliderFloat("Linear", &cbData_.attLin, 0.0001f, 4.0f, "%.4f");
			ImGui::SliderFloat("Quadratic", &cbData_.attQuad, 0.0000001f, 10.0f, "%.7f");

			if (ImGui::Button("Reset"))
			{
				Reset();
			}
		}
		ImGui::End();
	}

	/**
	 * \brief Initialize lighting parameters
	 */
	void PointLight::Reset() noexcept
	{
		cbData_ = {
			{10.0f, 9.0f, 2.5f},   // adjust point light initial position if needed
			{0.05f, 0.05f, 0.05f}, // turn down the ambient if needed
			{1.0f, 1.0f, 1.0f},
			1.0f,
			1.0f,
			0.045f,
			0.0075f,
		};
	}

	void PointLight::Draw(Graphics& gfx) const noxnd
	{
		mesh_.SetPos(cbData_.pos);
		mesh_.Draw(gfx);
	}

	/**
	 * \brief Update and bind point light constant buffer with latest position data
	 * and data from configurable IMGUI
	 * \param gfx Graphics object
	 * \param view View matrix, since we do lighting computation in camera space
	 */
	void PointLight::Bind(Graphics& gfx, DirectX::FXMMATRIX view) const noexcept
	{
		// compute light position in camera space
		auto dataCopy = cbData_;
		// to utilize SIMD, do computation using XMVECTOR
		const auto pos = DirectX::XMLoadFloat3(&cbData_.pos);
		DirectX::XMStoreFloat3(&dataCopy.pos, DirectX::XMVector3Transform(pos, view));
		cbuf_.Update(gfx, dataCopy);
		cbuf_.Bind(gfx);
	}
}
