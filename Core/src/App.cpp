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

	wnd_.Gfx().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 400.0f));
}

void App::DoFrame()
{
	const auto dt = timer_.Mark() * speedFactor_;
	wnd_.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd_.Gfx().SetCamera(cam_.GetMatrix());
	light_.Bind(wnd_.Gfx(), cam_.GetMatrix());

	// wall.Draw(wnd_.Gfx());
	// tp.Draw(wnd_.Gfx());
	// nano.Draw(wnd_.Gfx());
	// gobber.Draw(wnd_.Gfx());
	light_.Draw(wnd_.Gfx());
	sponza.Draw(wnd_.Gfx());

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

	// gobber.ShowWindow(wnd_.Gfx(), "gobber");
	// wall.ShowWindow(wnd_.Gfx(), "Wall");
	// tp.SpawnControlWindow(wnd_.Gfx());
	// nano.ShowWindow(wnd_.Gfx(), "Nano");

	sponza.ShowWindow(wnd_.Gfx(), "Sponza");
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
