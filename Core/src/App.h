#pragma once
#include "Camera.h"
#include "PointLight.h"
#include "Drawable/Complex/Mesh.h"
#include "Window/Window.h"
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
	void ShowModelWindow();

	ImguiManager imgui_;
	Window wnd_;
	Timer timer_;
	Camera cam_;
	float speedFactor_ = 1.0f;
	PointLight light_;
	Model nano{wnd_.GetGraphics(), "Models/nanosuit.obj"};
	struct
	{
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	} pos;
};
