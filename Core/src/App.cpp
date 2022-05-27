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

	while (const auto e = wnd_.kbd_.ReadKey())
	{
		if (e->IsPress() && e->GetCode() == VK_INSERT)
		{
			if (wnd_.CursorEnabled())
			{
				wnd_.DisableCursor();
				wnd_.mouse_.EnableRaw();
			}
			else
			{
				wnd_.EnableCursor();
				wnd_.mouse_.DisableRaw();
			}
		}
	}

	// imgui windows
	cam_.SpawnControlWindow();
	light_.SpawnControlWindow();
	nano.ShowWindow();
	ShowRawInputWindow();

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

void App::ShowRawInputWindow()
{
	while (const auto d = wnd_.mouse_.ReadRawDelta())
	{
		x += d->x;
		y += d->y;
	}
	if (ImGui::Begin("Raw Input"))
	{
		ImGui::Text("Tally: (%d,%d)", x, y);
		ImGui::Text("Cursor: %s", wnd_.CursorEnabled() ? "enabled" : "disabled");
	}
	ImGui::End();
}
