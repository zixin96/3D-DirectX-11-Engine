#pragma once
#include "Window/DXWindow.h"
#include "Utils/DXTimer.h"
#include "Imgui/ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"
#include "TestPlane.h"
#include "Drawable/Complex/Mesh.h"

/**
 * \brief This class is the top level of our application object,
 * owning the main window and top level game constructs
 */
class App
{
	public:
		App(const std::string& commandLine = "");
		// master frame / message loop
		int Go();
		~App() = default;
	private:
		void DoFrame();

		std::string         commandLine;
		ImguiManager        imgui_{};                                // always first initialize IMGUI
		D3DEngine::DXWindow wnd_{1920, 1080, "The Donkey Fart Box"}; // Please specify a resolution with aspect ratio = 16:9
		DXTimer             timer_{};
		float               speedFactor_{1.f};

		D3DEngine::Camera     cam_{};
		D3DEngine::PointLight light_{wnd_.Gfx()};
		// D3DEngine::Model      gobber{wnd_.Gfx(), "Models\\gobber\\GoblinX.obj", 6.0f};
		// D3DEngine::Model      wall{wnd_.Gfx(), "Models\\brick_wall\\brick_wall.obj", 6.0f};
		// D3DEngine::TestPlane  tp{wnd_.Gfx(), 6.0};
		// D3DEngine::Model      nano{wnd_.Gfx(), "Models\\nano_textured\\nanosuit.obj", 2.0f};
		D3DEngine::Model sponza{wnd_.Gfx(), "Models\\sponza\\sponza.obj", 1.0f / 20.0f};
};
