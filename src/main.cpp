// main.cpp

#include <atomic>
#include <iostream>
#include <thread>

import Dreamhearth;
import DreamhearthWindow;

import Input;
import Scene;
import SceneManager;

constexpr char const * AppName = "Sniff the Way - A Tail to Guide You Home";

namespace dh = Dreamhearth;

void on_error(std::string msg)
{
	std::cout << msg << std::endl;
}

int main()
{
	std::cout << "Initializing app..." << std::endl;

	dh::Window window(dh::WindowSize{ 1920, 1080 }, AppName, on_error);
	if (!window.IsValid())
		return -1;

	// these are synchronized across update/render thread and main event loop thread
	Input input;
	std::atomic<dh::WindowSize> window_size_pixels = window.GetWindowSizePixels(); // must only be called from main thread
	std::atomic<float> window_scale_factor = window.GetWindowScaleFactor(); // must only be called from main thread

	window.SetOnSizeChanged([&window_size_pixels](int width_pixels, int height_pixels)
		{
			window_size_pixels.store(dh::WindowSize{ width_pixels, height_pixels });
		});
	window.SetOnScaleFactorChanged([&window_scale_factor](float scale_factor)
		{
			window_scale_factor.store(scale_factor);
		});
	window.SetOnKeyEvent([&input](int key, int /*scan_code*/, int action, int /*mods*/)
		{
			if (action == static_cast<int>(Input::Action::Press))
				input.SetKey(key, true /*pressed*/);
			else if (action == static_cast<int>(Input::Action::Release))
				input.SetKey(key, false /*pressed*/);
		});

	std::cout << "Running app..." << std::endl;

	std::jthread update_render_loop(
		[&window, &input, &window_size_pixels, &window_scale_factor](std::stop_token s_token)
		{
			dh::WindowSize size = window_size_pixels.load();
			float scale_factor = window_scale_factor.load();

			dh::RenderContext render_context = window.CreateRenderContext(size);

			auto initial_scene = std::make_unique<Scene>(render_context, AppName);
			SceneManager scene_manager{ render_context, std::move(initial_scene) };
			scene_manager.OnDPIScaleFactorChanged(scale_factor);
			scene_manager.OnViewportResized(size.width, size.height);

			auto last_update_time = std::chrono::steady_clock::now();

			while (!s_token.stop_requested())
			{
				auto cur_time = std::chrono::steady_clock::now();
				float dt = std::chrono::duration<float>(cur_time - last_update_time).count(); // seconds
				last_update_time = cur_time;

				input.NewFrame();
				if (!scene_manager.Update(dt, input))
					break;

				dh::DrawFrameResult draw_result = window.DrawFrame(render_context, [&scene_manager]() { scene_manager.Render(); });

				if (draw_result == dh::DrawFrameResult::SurfaceLost)
					break; // The Cosmic compositor has issues

				dh::WindowSize new_size = window_size_pixels.load();
				if (draw_result == dh::DrawFrameResult::SwapChainOutOfDate || new_size != size)
				{
					render_context.SetViewportSize(new_size.width, new_size.height);
					scene_manager.OnViewportResized(new_size.width, new_size.height);
					size = new_size;
				}

				float new_scale_factor = window_scale_factor.load();
				if (new_scale_factor != scale_factor)
				{
					scene_manager.OnDPIScaleFactorChanged(new_scale_factor);
					scale_factor = new_scale_factor;
				}

				scene_manager.ApplyPendingTransition();
			}

			render_context.WaitForLastFrame();
			window.SetShouldClose(true); // signal main thread to exit
		}); // the RenderContext and SceneManager are destroyed in the reverse order

	while (!window.ShouldClose())
		window.PollEvents(); // must only be called from main thread
}
