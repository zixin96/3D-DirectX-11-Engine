#include "App.h"
#include "Utils/EngineMath.h"
#include "Utils/Surface.h"
#include "Utils/GDIPlusManager.h"
#include "imgui/imgui.h"

GDIPlusManager gdipm;

namespace dx = DirectX;

App::App()
	:
	wnd_(1600, 1200, "The Donkey Fart Box"),
	light_(wnd_.GetGraphics())
{
	wnd_.GetGraphics().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
}

int App::Go()
{
	while (true)
	{
		// process all pending messages, but do not block if there are no messages to process
		if (const auto ecode = Window::ProcessMessages())
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}

void App::ShowModelWindow()
{
	if (ImGui::Begin("Model"))
	{
		using namespace std::string_literals;

		ImGui::Text("Orientation");
		ImGui::SliderAngle("Roll", &pos.roll, -180.0f, 180.0f);
		ImGui::SliderAngle("Pitch", &pos.pitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &pos.yaw, -180.0f, 180.0f);

		ImGui::Text("Position");
		ImGui::SliderFloat("X", &pos.x, -20.0f, 20.0f);
		ImGui::SliderFloat("Y", &pos.y, -20.0f, 20.0f);
		ImGui::SliderFloat("Z", &pos.z, -20.0f, 20.0f);
	}
	ImGui::End();
}

void App::DoFrame()
{
	const auto dt = timer_.Mark() * speedFactor_;;
	wnd_.GetGraphics().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd_.GetGraphics().SetCamera(cam_.GetMatrix());
	light_.Bind(wnd_.GetGraphics(), cam_.GetMatrix());

	const auto transform = dx::XMMatrixRotationRollPitchYaw(pos.roll, pos.pitch, pos.yaw)
		* dx::XMMatrixTranslation(pos.x, pos.y, pos.z);
	nano.Draw(wnd_.GetGraphics(), transform);
	light_.Draw(wnd_.GetGraphics());

	// imgui windows
	cam_.SpawnControlWindow();
	light_.SpawnControlWindow();
	ShowModelWindow();

	// present
	wnd_.GetGraphics().EndFrame();
}
