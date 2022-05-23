#pragma once
#include "Camera.h"
#include "PointLight.h"
#include "Window/Window.h"
#include "Drawable/Box.h"
#include "Utils/Timer.h"
#include "Imgui/ImguiManager.h"
#include <set>

class App
{
public:
	App();
	// master frame / message loop
	int Go();

	~App() = default;

private:
	void DoFrame();
	void SpawnSimulationWindow() noexcept;
	void SpawnBoxWindowManagerWindow() noexcept;
	void SpawnBoxWindows() noexcept;

	ImguiManager imgui_;
	Window wnd_;
	Timer timer_;
	Camera cam_;
	std::vector<std::unique_ptr<Drawable>> drawables_;
	std::vector<Box*> boxes_;
	float speedFactor_ = 1.0f;
	PointLight light_;
	static constexpr size_t nDrawables_ = 180;
	std::optional<int> comboBoxIndex_;
	std::set<int> boxControlIds_;
};
