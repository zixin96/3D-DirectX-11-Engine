#include "App.h"
#include "Utils/GDIPlusManager.h"
#include "imgui/imgui.h"
#include <shellapi.h>
#include "Utils/TexturePreprocessor.h"

namespace dx = DirectX;

static GDIPlusManager gdipm;

App::App(const std::string& commandLine)
	: commandLine(commandLine)
{
	// makeshift cli for doing some preprocessing bullshit (so many hacks here)
	if (this->commandLine != "")
	{
		int        nArgs;
		const auto pLineW = GetCommandLineW();
		const auto pArgs  = CommandLineToArgvW(pLineW, &nArgs);
		if (nArgs >= 3 && std::wstring(pArgs[1]) == L"--twerk-objnorm")
		{
			const std::wstring pathInWide = pArgs[2];
			D3DEngine::TexturePreprocessor::FlipYAllNormalMapsInObj(
			                                                        std::string(pathInWide.begin(), pathInWide.end())
			                                                       );
			throw std::runtime_error("Normal maps all processed successfully. Just kidding about that whole runtime error thing.");
		}
		else if (nArgs >= 3 && std::wstring(pArgs[1]) == L"--twerk-flipy")
		{
			const std::wstring pathInWide  = pArgs[2];
			const std::wstring pathOutWide = pArgs[3];
			D3DEngine::TexturePreprocessor::FlipYNormalMap(
			                                               std::string(pathInWide.begin(), pathInWide.end()),
			                                               std::string(pathOutWide.begin(), pathOutWide.end())
			                                              );
			throw std::runtime_error("Normal map processed successfully. Just kidding about that whole runtime error thing.");
		}
		else if (nArgs >= 4 && std::wstring(pArgs[1]) == L"--twerk-validate")
		{
			const std::wstring minWide  = pArgs[2];
			const std::wstring maxWide  = pArgs[3];
			const std::wstring pathWide = pArgs[4];
			D3DEngine::TexturePreprocessor::ValidateNormalMap(
			                                                  std::string(pathWide.begin(), pathWide.end()), std::stof(minWide), std::stof(maxWide)
			                                                 );
			throw std::runtime_error("Normal map validated successfully. Just kidding about that whole runtime error thing.");
		}
	}
	//wall.SetRootTransform( dx::XMMatrixTranslation( -12.0f,0.0f,0.0f ) );
	//tp.SetPos( { 12.0f,0.0f,0.0f } );
	//gobber.SetRootTransform( dx::XMMatrixTranslation( 0.0f,0.0f,-4.0f ) );
	//nano.SetRootTransform( dx::XMMatrixTranslation( 0.0f,-7.0f,6.0f ) );
	bluePlane.SetPos(cam.GetPos());
	redPlane.SetPos(cam.GetPos());

	wnd.Gfx().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 400.0f)); // adjust the draw distance based on your scene
}

void App::DoFrame()
{
	const auto dt = timer_.Mark() * speedFactor_;
	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd.Gfx().SetCamera(cam.GetMatrix());
	light_.Bind(wnd.Gfx(), cam.GetMatrix());

	// wall.Draw(wnd_.Gfx());
	// tp.Draw(wnd_.Gfx());
	// nano.Draw(wnd_.Gfx());
	// gobber.Draw(wnd_.Gfx());
	light_.Draw(wnd.Gfx());
	sponza.Draw(wnd.Gfx());
	bluePlane.Draw(wnd.Gfx());
	redPlane.Draw(wnd.Gfx());

	while (const auto e = wnd.kbd_.ReadKey())
	{
		if (!e->IsPress())
		{
			continue;
		}

		switch (e->GetCode())
		{
			case VK_F1:
				if (wnd.CursorEnabled())
				{
					wnd.DisableCursor();
					wnd.mouse_.EnableRaw();
				}
				else
				{
					wnd.EnableCursor();
					wnd.mouse_.DisableRaw();
				}
				break;
		}
	}

	if (!wnd.CursorEnabled())
	{
		if (wnd.kbd_.KeyIsPressed('W'))
		{
			cam.Translate({0.0f, 0.0f, dt});
		}
		if (wnd.kbd_.KeyIsPressed('A'))
		{
			cam.Translate({-dt, 0.0f, 0.0f});
		}
		if (wnd.kbd_.KeyIsPressed('S'))
		{
			cam.Translate({0.0f, 0.0f, -dt});
		}
		if (wnd.kbd_.KeyIsPressed('D'))
		{
			cam.Translate({dt, 0.0f, 0.0f});
		}
		if (wnd.kbd_.KeyIsPressed('R'))
		{
			cam.Translate({0.0f, dt, 0.0f});
		}
		if (wnd.kbd_.KeyIsPressed('F'))
		{
			cam.Translate({0.0f, -dt, 0.0f});
		}
	}

	while (const auto delta = wnd.mouse_.ReadRawDelta())
	{
		if (!wnd.CursorEnabled())
		{
			cam.Rotate((float)delta->x, (float)delta->y);
		}
	}

	// imgui windows
	cam.SpawnControlWindow();
	light_.SpawnControlWindow();

	// gobber.ShowWindow(wnd_.Gfx(), "gobber");
	// wall.ShowWindow(wnd_.Gfx(), "Wall");
	// tp.SpawnControlWindow(wnd_.Gfx());
	// nano.ShowWindow(wnd_.Gfx(), "Nano");
	sponza.ShowWindow(wnd.Gfx(), "Sponza");
	bluePlane.SpawnControlWindow(wnd.Gfx(), "Blue Plane");
	redPlane.SpawnControlWindow(wnd.Gfx(), "Red Plane");

	// present
	wnd.Gfx().EndFrame();
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
