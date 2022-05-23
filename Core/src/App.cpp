#include "App.h"

#include "Drawable/Melon.h"
#include "Drawable/Pyramid.h"
#include "Drawable/Sheet.h"
#include "Drawable/SkinnedBox.h"
#include "Utils/EngineMath.h"
#include "Utils/Surface.h"
#include "Utils/GDIPlusManager.h"
#include "Drawable/Cylinder.h"
#include "imgui/imgui.h"

GDIPlusManager gdipm;

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
			const DirectX::XMFLOAT3 mat = {cdist(rng), cdist(rng), cdist(rng)};

			switch (sdist(rng))
			{
			case 0:
				return std::make_unique<Box>(
					gfx, rng, adist, ddist,
					odist, rdist, bdist, mat
				);
			case 1:
				return std::make_unique<Cylinder>(
					gfx, rng, adist, ddist, odist,
					rdist, bdist, tdist
				);
			case 2:
				return std::make_unique<Pyramid>(
					gfx, rng, adist, ddist, odist,
					rdist, tdist
				);
			case 3:
				return std::make_unique<SkinnedBox>(
					gfx, rng, adist, ddist,
					odist, rdist
				);
			default:
				assert(false && "impossible drawable option in factory");
				return {};
			}

			// select which drawable to create
			/*
			const DirectX::XMFLOAT3 mat = {cdist(rng), cdist(rng), cdist(rng)};

			return std::make_unique<Box>(
				gfx, rng, adist, ddist,
				odist, rdist, bdist, mat
			);
			switch (typedist(rng))
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
		std::uniform_int_distribution<int> sdist{0, 3};
		std::uniform_real_distribution<float> adist{0.0f, PI * 2.0f};
		std::uniform_real_distribution<float> ddist{0.0f, PI * 0.5f};
		std::uniform_real_distribution<float> odist{0.0f, PI * 0.08f};
		std::uniform_real_distribution<float> rdist{6.0f, 20.0f};
		std::uniform_real_distribution<float> bdist{0.4f, 3.0f};
		std::uniform_int_distribution<int> latdist{5, 20};
		std::uniform_int_distribution<int> longdist{10, 40};
		std::uniform_real_distribution<float> cdist{0.0f, 1.0f};
		std::uniform_int_distribution<int> tdist{3, 30};
		// std::uniform_int_distribution<int> typedist{0, 4};
	};


	drawables_.reserve(nDrawables_);
	std::generate_n(std::back_inserter(drawables_), nDrawables_, Factory{wnd_.GetGraphics()});

	// init box pointers for editing instance parameters
	for (auto& pd : drawables_)
	{
		// only want boxes_
		if (auto pb = dynamic_cast<Box*>(pd.get()))
		{
			boxes_.push_back(pb);
		}
	}

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
	light_.Bind(wnd_.GetGraphics(), cam_.GetMatrix());

	for (auto& d : drawables_)
	{
		d->Update(wnd_.kbd_.KeyIsPressed(VK_SPACE) ? 0.0f : dt);
		d->Draw(wnd_.GetGraphics());
	}
	light_.Draw(wnd_.GetGraphics());

	// imgui window to control simulation speed
	SpawnSimulationWindow();

	// imgui window to control camera
	cam_.SpawnControlWindow();

	// imgui window to contrl light
	light_.SpawnControlWindow();

	// imgui window to open box windows
	SpawnBoxWindowManagerWindow();

	// imgui box attribute control windows
	SpawnBoxWindows();

	// present
	wnd_.GetGraphics().EndFrame();
}

void App::SpawnSimulationWindow() noexcept
{
	if (ImGui::Begin("Simulation Speed"))
	{
		ImGui::SliderFloat("Speed Factor", &speedFactor_, 0.0f, 6.0f, "%.4f");
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Status: %s", wnd_.kbd_.KeyIsPressed(VK_SPACE) ? "PAUSED" : "RUNNING");
	}
	ImGui::End();
}

void App::SpawnBoxWindowManagerWindow() noexcept
{
	if (ImGui::Begin("Boxes"))
	{
		using namespace std::string_literals;
		const auto preview = comboBoxIndex_ ? std::to_string(*comboBoxIndex_) : "Choose a box..."s;
		if (ImGui::BeginCombo("Box Number", preview.c_str()))
		{
			for (int i = 0; i < boxes_.size(); i++)
			{
				// const bool selected = *comboBoxIndex_ == i;
				if (ImGui::Selectable(std::to_string(i).c_str()))
				{
					comboBoxIndex_ = i;
				}
				/*if (selected)
				{
					ImGui::SetItemDefaultFocus();
				}*/
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Spawn Control Window") && comboBoxIndex_)
		{
			boxControlIds_.insert(*comboBoxIndex_);
			comboBoxIndex_.reset();
		}
	}
	ImGui::End();
}

void App::SpawnBoxWindows() noexcept
{
	for (auto i = boxControlIds_.begin(); i != boxControlIds_.end();)
	{
		if (!boxes_[*i]->SpawnControlWindow(*i, wnd_.GetGraphics()))
		{
			i = boxControlIds_.erase(i);
		}
		else
		{
			i++;
		}
	}
}
