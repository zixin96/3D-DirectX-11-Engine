#pragma once
#include "Window/DXWindow.h"
#include "Utils/DXTimer.h"
#include "Imgui/ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"

/**
 * \brief This class is the top level of our application object,
 * owning the main window and top level game constructs
 */
class AppAssortmentPhong
{
	public:
		AppAssortmentPhong();
		// master frame / message loop
		int Go();
		~AppAssortmentPhong() = default;
	private:
		void SpawnSimulationWindow() noxnd;
		void SpawnBoxWindowManagerWindow() noxnd;
		void SpawnBoxWindows() noxnd;
		void DoFrame();

		// always first initialize IMGUI
		// Please specify a resolution with aspect ratio = 16:9

		ImguiManager                           imgui_{};
		DXWindow                               wnd_{1920, 1080, "The Donkey Fart Box"};
		DXTimer                                timer_{};
		float                                  speedFactor_{1.f};
		Camera                                 cam_{};
		PointLight                             light_{wnd_.Gfx()};
		std::vector<std::unique_ptr<Drawable>> drawables_{};
		static constexpr size_t                nDrawables_ = 180;

		// The following parameters are used to control individual boxes: 
		// keep track of drawables that are boxes

		std::vector<class Box*> boxes_{};
		std::optional<int>      comboBoxIndex_;
		std::set<int>           boxControlIds_;
};
