#pragma once
#include "Imgui/ImguiManager.h"
#include "Window/DXWindow.h"
#include "Utils/DXTimer.h"
#include "Camera.h"

/**
 * \brief This class is the top level of our application object,
 * owning the main window and top level game constructs
 */
class AppAssortment
{
public:
	AppAssortment();
	// master frame / message loop
	int Go();
	~AppAssortment() = default;
	
private:
	void SpawnSimulationWindow() noxnd;
	void DoFrame();

	// always first initialize IMGUI
	ImguiManager imgui_{};
	// Please specify a resolution with aspect ratio = 16:9
	DXWindow wnd{ 1920, 1080, "The Donkey Fart Box"};
	DXTimer timer{};
	float speedFactor_{ 1.f };
	Camera cam_{};

	std::vector<std::unique_ptr<class Drawable>> drawables{};
	static constexpr size_t nDrawables = 180;
};
