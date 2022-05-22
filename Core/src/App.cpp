#include "App.h"
#include <memory>

#include "Drawable/Melon.h"
#include "Drawable/Pyramid.h"
#include "Drawable/Sheet.h"
#include "Drawable/SkinnedBox.h"
#include "Utils/EngineMath.h"
#include "Utils/Surface.h"
// #include "Utils/GDIPlusManager.h"
#include "imgui/imgui.h"

// GDIPlusManager gdipm;

namespace dx = DirectX;

App::App()
	:
	wnd_(1600, 1200, "The Donkey Fart Box"),
	light_(wnd_.GetGraphics())
{
	class Factory
	{
	public:
		Factory(Graphics& gfx)
			:
			gfx(gfx)
		{
		}

		std::unique_ptr<Drawable> operator()()
		{
			return std::make_unique<Box>(
				gfx, rng, adist, ddist,
				odist, rdist, bdist
			);
			// select which drawable to create
			/*switch (typedist(rng))
			{
			case 0:
				return std::make_unique<Pyramid>(
					gfx, rng, adist, ddist,
					odist, rdist
				);
			case 1:
				return std::make_unique<Box>(
					gfx, rng, adist, ddist,
					odist, rdist, bdist
				);
			case 2:
				return std::make_unique<Melon>(
					gfx, rng, adist, ddist,
					odist, rdist, longdist, latdist
				);
			case 3:
				return std::make_unique<Sheet>(
					gfx, rng, adist, ddist,
					odist, rdist
				);
			case 4:
				return std::make_unique<SkinnedBox>(
					gfx, rng, adist, ddist,
					odist, rdist
				);
			default:
				assert(false && "bad drawable type in factory");
				return {};
			}*/
		}

	private:
		Graphics& gfx;
		std::mt19937 rng{std::random_device{}()};
		std::uniform_real_distribution<float> adist{0.0f, PI * 2.0f};
		std::uniform_real_distribution<float> ddist{0.0f, PI * 0.5f};
		std::uniform_real_distribution<float> odist{0.0f, PI * 0.08f};
		std::uniform_real_distribution<float> rdist{6.0f, 20.0f};
		std::uniform_real_distribution<float> bdist{0.4f, 3.0f};
		std::uniform_int_distribution<int> latdist{5, 20};
		std::uniform_int_distribution<int> longdist{10, 40};
		// std::uniform_int_distribution<int> typedist{0, 4};
	};

	// Factory f(wnd_.GetGraphics());
	// drawables_.reserve(nDrawables_);
	// std::generate_n(std::back_inserter(drawables_), nDrawables_, f);

	drawables_.reserve(nDrawables_);
	std::generate_n(std::back_inserter(drawables_), nDrawables_, Factory{wnd_.GetGraphics()});
	wnd_.GetGraphics().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
}

int App::Go()
{
	while (true)
	{
		// process all pending messages, but do not block if there are no messages to process
		if (const auto ecode = Window::ProcessMessages())
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}

void App::DoFrame()
{
	const auto dt = timer_.Mark() * speedFactor_;;
	wnd_.GetGraphics().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd_.GetGraphics().SetCamera(cam_.GetMatrix());
	light_.Bind(wnd_.GetGraphics());

	for (auto& d : drawables_)
	{
		d->Update(wnd_.kbd_.KeyIsPressed(VK_SPACE) ? 0.0f : dt);
		d->Draw(wnd_.GetGraphics());
	}
	light_.Draw(wnd_.GetGraphics());

	static char buffer[1024];

	// imgui window to control simulation speed
	if (ImGui::Begin("Simulation Speed"))
	{
		ImGui::SliderFloat("Speed Factor", &speedFactor_, 0.0f, 4.0f);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Status: %s", wnd_.kbd_.KeyIsPressed(VK_SPACE) ? "PAUSED" : "RUNNING");
	}
	ImGui::End();

	// imgui window to control camera
	cam_.SpawnControlWindow();

	// imgui window to contrl light
	light_.SpawnControlWindow();

	// present
	wnd_.GetGraphics().EndFrame();
}
