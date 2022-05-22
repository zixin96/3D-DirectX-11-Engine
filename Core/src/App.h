#pragma once
#include "Camera.h"
#include "PointLight.h"
#include "Window/Window.h"
#include "Drawable/Box.h"
#include "Utils/Timer.h"
#include "Imgui/ImguiManager.h"

class App
{
public:
	App();
	// master frame / message loop
	int Go();

	~App() = default;

private:
	void DoFrame();
	ImguiManager imgui_;
	Window wnd_;
	Timer timer_;
	Camera cam_;
	std::vector<std::unique_ptr<Drawable>> drawables_;
	float speedFactor_ = 1.0f;
	PointLight light_;
	static constexpr size_t nDrawables_ = 180;
};
