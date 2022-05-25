#include "App.h"
#include "Utils/EngineMath.h"
#include "Utils/Surface.h"
#include "Utils/GDIPlusManager.h"
#include "imgui/imgui.h"

namespace dx = DirectX;

GDIPlusManager gdipm;

App::App()
	:
	// Please specify a resolution with aspect ratio = 16:9
	wnd_(1920, 1080, "The Donkey Fart Box"),
	light_(wnd_.GetGraphics())
{
	wnd_.GetGraphics().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
}

void App::DoFrame()
{
	const auto dt = timer_.Mark() * speedFactor_;;
	wnd_.GetGraphics().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd_.GetGraphics().SetCamera(cam_.GetMatrix());
	light_.Bind(wnd_.GetGraphics(), cam_.GetMatrix());

	nano.Draw(wnd_.GetGraphics());
	light_.Draw(wnd_.GetGraphics());

	// imgui windows
	cam_.SpawnControlWindow();
	light_.SpawnControlWindow();
	// ShowImguiDemoWindow();
	nano.ShowWindow();

	// present
	wnd_.GetGraphics().EndFrame();
}

void App::ShowImguiDemoWindow()
{
	static bool show_demo_window = true;
	if (show_demo_window)
	{
		ImGui::ShowDemoWindow(&show_demo_window);
	}
}

App::~App()
{
}

int App::Go()
{
	while (true)
	{
		// process all pending messages, but do not block if there are no messages to process
		if (const auto ecode = DXWindow::ProcessMessages())
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}
