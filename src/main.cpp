// main.cpp

#include <atomic>
#include <chrono>
#include <thread>
#include <utility>

#include <glog/logging.h>

import Dreamhearth;
import DreamhearthWindow;

import AudioSystem;
import Input;
import IScene;
import SceneManager;
import SniffTheWayConstants;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

namespace
{
	class LoggingLifetime
	{
	public:
		explicit LoggingLifetime(char const * executable_name)
		{
#ifndef NDEBUG
			FLAGS_logtostderr = true;
#endif
			google::InitGoogleLogging(executable_name);
		}

		~LoggingLifetime()
		{
			google::ShutdownGoogleLogging();
		}

		LoggingLifetime(LoggingLifetime const &) = delete;
		LoggingLifetime & operator=(LoggingLifetime const &) = delete;
	};
}

void on_error(std::string msg)
{
	LOG(ERROR) << msg;
}

void on_graphics_diagnostic(dh::GraphicsDiagnostic const & diagnostic)
{
	switch (diagnostic.severity)
	{
	case dh::GraphicsDiagnosticSeverity::Info:
		LOG(INFO) << diagnostic.message;
		break;
	case dh::GraphicsDiagnosticSeverity::Warning:
		LOG(WARNING) << diagnostic.message;
		break;
	case dh::GraphicsDiagnosticSeverity::Error:
		LOG(ERROR) << diagnostic.message;
		break;
	}
}

void run_update_render_loop(
	dh::Window const & window,
	Input & input,
	std::atomic<dh::WindowSize> const & window_size_pixels,
	std::stop_token const & s_token)
{
	dh::WindowSize last_window_size = window_size_pixels.load();

	auto render_context_result = window.CreateRenderContext(last_window_size, on_graphics_diagnostic);
	if (!render_context_result)
	{
		on_error(render_context_result.error().GetMessage());
		window.SetShouldClose(true);
		return;
	}
	dh::RenderContext render_context = std::move(render_context_result).value();

	AudioSystem audio_system;
	SceneManager scene_manager{audio_system, render_context, SceneTransition{ SceneId::Title }};
	dh::RenderExtent render_extent = render_context.GetSwapChainExtent();
	scene_manager.OnWindowResized(render_extent.width, render_extent.height);

	auto last_update_time = std::chrono::steady_clock::now();
	bool swap_chain_needs_recreation = false;

	while (!s_token.stop_requested())
	{
		dh::WindowSize const window_size = window_size_pixels.load();
		if (swap_chain_needs_recreation || window_size != last_window_size)
		{
			last_window_size = window_size;
			auto recreate_result = render_context.RecreateSwapChain(window_size.width, window_size.height);
			if (!recreate_result)
			{
				on_error(recreate_result.error().GetMessage());
				break;
			}

			render_extent = render_context.GetSwapChainExtent();
			swap_chain_needs_recreation = render_extent.width <= 0 || render_extent.height <= 0;
			if (!swap_chain_needs_recreation)
				scene_manager.OnWindowResized(render_extent.width, render_extent.height);
		}

		// If the window is minimized, the swap chain extent is invalid. Wait until the window is restored before continuing.
		if (swap_chain_needs_recreation)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds{ 16 });
			last_update_time = std::chrono::steady_clock::now();
			continue;
		}

		auto cur_time = std::chrono::steady_clock::now();
		float dt = std::chrono::duration<float>(cur_time - last_update_time).count(); // seconds
		last_update_time = cur_time;

		input.NewFrame();
		scene_manager.Update(dt, input);

		dh::DrawFrameResult draw_result = window.DrawFrame(render_context, [&scene_manager]() { scene_manager.Render(); });

		if (draw_result == dh::DrawFrameResult::SurfaceLost)
			break; // The Cosmic compositor has issues

		if (draw_result == dh::DrawFrameResult::SwapChainOutOfDate)
			swap_chain_needs_recreation = true;

		if (!scene_manager.ApplyPendingTransition())
			break;
	}

	render_context.WaitForLastFrame();
	window.SetShouldClose(true); // signal main thread to exit
}

int main(int argc, char * argv[])
{
	LoggingLifetime logging{ argc > 0 && argv[0] ? argv[0] : FullTitle };

	LOG(INFO) << "Initializing app...";

	dh::Window window(dh::WindowSize{ 2560, 1440 }, FullTitle, on_error);
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
	window.SetOnMouseButtonEvent([&input](int button, int action, int /*mods*/)
		{
			if (action == static_cast<int>(Input::Action::Press))
				input.SetMouseButton(button, true /*pressed*/);
			else if (action == static_cast<int>(Input::Action::Release))
				input.SetMouseButton(button, false /*pressed*/);
		});
	window.SetOnCursorPos([&input](float x_pixels, float y_pixels)
		{
			input.SetMousePos(x_pixels, y_pixels);
		});

	LOG(INFO) << "Running app...";

	std::jthread update_render_loop(
		[&window, &input, &window_size_pixels](std::stop_token s_token)
		{
			run_update_render_loop(window, input, window_size_pixels, s_token);
		});

	while (!window.ShouldClose())
		window.PollEvents(); // must only be called from main thread
}
