// main.cpp

#include <atomic>
#include <iostream>
#include <thread>

import Dreamhearth;
import DreamhearthWindow;
namespace dh = Dreamhearth;

import Input;
import SceneManager;
import SniffTheWayConstants;

using namespace SniffTheWay;

void on_error(std::string msg)
{
	std::cout << msg << std::endl;
}

void run_update_render_loop(
	dh::Window const & window,
	Input & input,
	std::atomic<dh::WindowSize> const & window_size_pixels,
	std::stop_token const & s_token)
{
	dh::WindowSize size = window_size_pixels.load();

	dh::RenderContext render_context = window.CreateRenderContext(size);

	SceneManager scene_manager{render_context, SceneTransition{ SceneId::Picnic }};
	scene_manager.OnWindowResized(size.width, size.height);

	auto last_update_time = std::chrono::steady_clock::now();

	while (!s_token.stop_requested())
	{
		auto cur_time = std::chrono::steady_clock::now();
		float dt = std::chrono::duration<float>(cur_time - last_update_time).count(); // seconds
		last_update_time = cur_time;

		input.NewFrame();
		scene_manager.Update(dt, input);

		dh::DrawFrameResult draw_result = window.DrawFrame(render_context, [&scene_manager]() { scene_manager.Render(); });

		if (draw_result == dh::DrawFrameResult::SurfaceLost)
			break; // The Cosmic compositor has issues

		dh::WindowSize new_size = window_size_pixels.load();
		if (draw_result == dh::DrawFrameResult::SwapChainOutOfDate || new_size != size)
		{
			render_context.RecreateSwapChain(new_size.width, new_size.height);
			scene_manager.OnWindowResized(new_size.width, new_size.height);
			size = new_size;
		}

		if (!scene_manager.ApplyPendingTransition())
			break;
	}

	render_context.WaitForLastFrame();
	window.SetShouldClose(true); // signal main thread to exit
}

int main()
{
	std::cout << "Initializing app..." << std::endl;

	dh::Window window(dh::WindowSize{ static_cast<int>(UIWidth), static_cast<int>(UIHeight) }, FullTitle, on_error);
	if (!window.IsValid())
		return -1;

	// these are synchronized across update/render thread and main event loop thread
	Input input;
	std::atomic<dh::WindowSize> window_size_pixels = window.GetWindowSizePixels(); // must only be called from main thread

	window.SetOnSizeChanged([&window_size_pixels](int width_pixels, int height_pixels)
		{
			window_size_pixels.store(dh::WindowSize{ width_pixels, height_pixels });
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
		[&window, &input, &window_size_pixels](std::stop_token s_token)
		{
			run_update_render_loop(window, input, window_size_pixels, s_token);
		});

	while (!window.ShouldClose())
		window.PollEvents(); // must only be called from main thread
}
