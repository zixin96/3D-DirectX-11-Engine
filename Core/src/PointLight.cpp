#include "PointLight.h"
#include "imgui/imgui.h"

PointLight::PointLight(Graphics& gfx, float radius)
	:
	mesh_(gfx, radius),
	cbuf_(gfx)
{
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

void PointLight::Reset() noexcept
{
	cbData_ = {
		{0.0f, 0.0f, 0.0f},
		{0.05f, 0.05f, 0.05f},
		{1.0f, 1.0f, 1.0f},
		1.0f,
		1.0f,
		0.045f,
		0.0075f,
	};
}

void PointLight::Draw(Graphics& gfx) const noexcept(!IS_DEBUG)
{
	mesh_.SetPos(cbData_.pos);
	mesh_.Draw(gfx);
}

void PointLight::Bind(Graphics& gfx) const noexcept
{
	cbuf_.Update(gfx, cbData_);
	cbuf_.Bind(gfx);
}
