#include "App.h"
#include "Utils/GDIPlusManager.h"
#include "imgui/imgui.h"

namespace dx = DirectX;

static GDIPlusManager gdipm;

App::App()
{
	wnd_.Gfx().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
}

void App::DoFrame()
{
	const auto dt = timer_.Mark() * speedFactor_;
	wnd_.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd_.Gfx().SetCamera(cam_.GetMatrix());
	light_.Bind(wnd_.Gfx(), cam_.GetMatrix());

	nano.Draw(wnd_.Gfx());
	light_.Draw(wnd_.Gfx());

	// imgui windows
	cam_.SpawnControlWindow();
	light_.SpawnControlWindow();
	nano.ShowWindow();

	// present
	wnd_.Gfx().EndFrame();
}

int App::Go()
{
	while (true)
	{
		// process all pending messages, but do not block if there are no messages to process
		if (const auto ecode = DXWindow::ProcessMessages())
		{
			// if return std::optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}
