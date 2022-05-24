#pragma once
// TODO: Change this window file name
#include "Window/Window.h"
#include "Utils/Timer.h"
#include "Imgui/ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"
#include "Drawable/Complex/Mesh.h"

class App
{
public:
	App();
	// master frame / message loop
	int Go();
	~App();
private:
	void DoFrame();
	void ShowImguiDemoWindow();

	ImguiManager imgui_;
	Window wnd_;
	Timer timer_;
	float speedFactor_ = 1.0f;
	Camera cam_;
	PointLight light_;
	Model nano{wnd_.GetGraphics(), "Models/nano.gltf"};
};
