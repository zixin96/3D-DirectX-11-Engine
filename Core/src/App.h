#pragma once
#include "Window/DXWindow.h"
#include "Utils/DXTimer.h"
#include "Imgui/ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"
#include "TestCube.h"
#include "TestPlane.h"
#include "Drawable/Complex/Mesh.h"

/**
 * \brief This class is the top level of our application object,
 * owning the main window and top level game constructs
 */
class App
{
	public:
		App();
		// master frame / message loop
		int Go();
		~App() = default;
	private:
		void DoFrame();
		// always first initialize IMGUI
		ImguiManager imgui_{};
		// Please specify a resolution with aspect ratio = 16:9
		D3DEngine::DXWindow   wnd_{1920, 1080, "The Donkey Fart Box"};
		DXTimer               timer_{};
		float                 speedFactor_{1.f};
		Camera                cam_{};
		D3DEngine::PointLight light_{wnd_.Gfx()};
		D3DEngine::Model      wall_{wnd_.Gfx(), "Models/brick_wall/brick_wall.obj"};
		D3DEngine::TestPlane tp_{wnd_.Gfx(), 1.0};
		D3DEngine::TestCube  cube_{wnd_.Gfx(), 2.0};
};
