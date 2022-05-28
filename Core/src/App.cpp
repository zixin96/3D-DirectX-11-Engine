#include "App.h"
#include "Utils/GDIPlusManager.h"
#include "imgui/imgui.h"

namespace dx = DirectX;

static GDIPlusManager gdipm;

App::App()
{
	// wall_.SetRootTransform(dx::XMMatrixTranslation(5.0f, 0.0f, 0.0f));
	// tp_.SetPos({1.5f, 0.0f, 0.0f});
	// cube_.SetPos({-1.5f, 0.0f, 0.0f});

	wnd_.Gfx().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
}

void App::DoFrame()
{
	const auto dt = timer_.Mark() * speedFactor_;
	wnd_.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd_.Gfx().SetCamera(cam_.GetMatrix());
	light_.Bind(wnd_.Gfx(), cam_.GetMatrix());

	// wall_.Draw(wnd_.Gfx());
	// tp_.Draw(wnd_.Gfx());
	// cube_.Draw(wnd_.Gfx());

	gobber.Draw(wnd_.Gfx());
	light_.Draw(wnd_.Gfx());

	while (const auto e = wnd_.kbd_.ReadKey())
	{
		if (!e->IsPress())
		{
			continue;
		}

		switch (e->GetCode())
		{
			case VK_F1:
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
				break;
		}
	}

	if (!wnd_.CursorEnabled())
	{
		if (wnd_.kbd_.KeyIsPressed('W'))
		{
			cam_.Translate({0.0f, 0.0f, dt});
		}
		if (wnd_.kbd_.KeyIsPressed('A'))
		{
			cam_.Translate({-dt, 0.0f, 0.0f});
		}
		if (wnd_.kbd_.KeyIsPressed('S'))
		{
			cam_.Translate({0.0f, 0.0f, -dt});
		}
		if (wnd_.kbd_.KeyIsPressed('D'))
		{
			cam_.Translate({dt, 0.0f, 0.0f});
		}
		if (wnd_.kbd_.KeyIsPressed('R'))
		{
			cam_.Translate({0.0f, dt, 0.0f});
		}
		if (wnd_.kbd_.KeyIsPressed('F'))
		{
			cam_.Translate({0.0f, -dt, 0.0f});
		}
	}

	while (const auto delta = wnd_.mouse_.ReadRawDelta())
	{
		if (!wnd_.CursorEnabled())
		{
			cam_.Rotate((float)delta->x, (float)delta->y);
		}
	}

	// imgui windows
	cam_.SpawnControlWindow();
	light_.SpawnControlWindow();
	// wall_.ShowWindow("Wall");
	// tp_.SpawnControlWindow(wnd_.Gfx());
	// cube_.SpawnControlWindow(wnd_.Gfx());
	gobber.ShowWindow(wnd_.Gfx(), "gobber");

	// present
	wnd_.Gfx().EndFrame();
}

int App::Go()
{
	while (true)
	{
		// process all pending messages, but do not block if there are no messages to process
		if (const auto ecode = D3DEngine::DXWindow::ProcessMessages())
		{
			// if return std::optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}
