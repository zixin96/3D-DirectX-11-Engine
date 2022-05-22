#include "PointLight.h"
#include "imgui/imgui.h"

PointLight::PointLight(Graphics& gfx, float radius)
	:
	mesh_(gfx, radius),
	cbuf_(gfx)
{
}

void PointLight::SpawnControlWindow() noexcept
{
	if (ImGui::Begin("Light"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &pos_.x, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Y", &pos_.y, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Z", &pos_.z, -60.0f, 60.0f, "%.1f");
		if (ImGui::Button("Reset"))
		{
			Reset();
		}
	}
	ImGui::End();
}

void PointLight::Reset() noexcept
{
	pos_ = {0.0f, 0.0f, 0.0f};
}

void PointLight::Draw(Graphics& gfx) const noexcept(!IS_DEBUG)
{
	mesh_.SetPos(pos_);
	mesh_.Draw(gfx);
}

void PointLight::Bind(Graphics& gfx) const noexcept
{
	cbuf_.Update(gfx, PointLightCBuf{pos_});
	cbuf_.Bind(gfx);
}
