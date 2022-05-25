#include "AppAssortment.h"

#include <imgui.h>

#include "Drawable/Geometry/Melon.h"
#include "Drawable/Geometry/Pyramid.h"
#include "Drawable/Geometry/Box.h"
#include "Utils/EngineMath.h"

namespace dx = DirectX;

AppAssortment::AppAssortment()
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
			const DirectX::XMFLOAT3 mat = {cdist(rng), cdist(rng), cdist(rng)};

			switch (typedist(rng))
			{
			case 0:
				return std::make_unique<Pyramid>(gfx,
				                                 rng,
				                                 adist,
				                                 ddist,
				                                 odist,
				                                 rdist
				);
			case 1:
				return std::make_unique<Box>(gfx,
				                             rng,
				                             adist,
				                             ddist,
				                             odist,
				                             rdist,
				                             bdist
				);
			case 2:
				return std::make_unique<Melon>(gfx,
				                               rng,
				                               adist,
				                               ddist,
				                               odist,
				                               rdist,
				                               longdist,
				                               latdist
				);
			default:
				assert(false && "bad drawable type in factory");
				return {};
			}
		}

	private:
		Graphics& gfx;
		std::mt19937 rng{std::random_device{}()};
		std::uniform_real_distribution<float> adist{0.0f, PI * 2.0f};
		std::uniform_real_distribution<float> ddist{0.0f, PI * 0.5f};
		std::uniform_real_distribution<float> odist{0.0f, PI * 0.08f};
		std::uniform_real_distribution<float> rdist{6.0f, 20.0f};
		std::uniform_real_distribution<float> bdist{0.4f, 3.0f};
		std::uniform_real_distribution<float> cdist{0.0f, 1.0f};
		std::uniform_int_distribution<int> tdist{3, 30};
		std::uniform_int_distribution<int> latdist{5, 20};
		std::uniform_int_distribution<int> longdist{10, 40};
		std::uniform_int_distribution<int> typedist{0, 2};
	};

	Factory f(wnd.Gfx());
	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), nDrawables, f);

	wnd.Gfx().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
}

void AppAssortment::SpawnSimulationWindow() noexcept(!true)
{
	if (ImGui::Begin("Simulation Speed"))
	{
		ImGui::SliderFloat("Speed Factor", &speedFactor_, 0.0f, 6.0f, "%.4f");
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Status: %s", wnd.kbd_.KeyIsPressed(VK_SPACE) ? "PAUSED" : "RUNNING (hold spacebar to pause)");
	}
	ImGui::End();
}

void AppAssortment::DoFrame()
{
	const auto dt = timer.Mark() * speedFactor_;
	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd.Gfx().SetCamera(cam_.GetMatrix());
	for (auto& d : drawables)
	{
		d->Update(wnd.kbd_.KeyIsPressed(VK_SPACE) ? 0.0f : dt);
		d->Draw(wnd.Gfx());
	}
	SpawnSimulationWindow();
	cam_.SpawnControlWindow();
	wnd.Gfx().EndFrame();
}

int AppAssortment::Go()
{
	while (true)
	{
		// process all messages pending, but to not block for new messages
		if (const auto ecode = DXWindow::ProcessMessages())
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}
