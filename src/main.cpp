// main.cpp

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include <glog/logging.h>

import Dreamhearth;
import DreamhearthWindow;

import Game;
import Input;
import PlatformUtils;
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

std::optional<std::string> run_update_render_loop_impl(
	dh::Window const & window,
	Input & input,
	std::atomic<dh::WindowSize> const & window_size_pixels,
	std::stop_token const & s_token)
{
	dh::WindowSize last_window_size = window_size_pixels.load();

	auto render_context_result = window.CreateRenderContext(last_window_size, on_graphics_diagnostic);
	if (!render_context_result)
		return render_context_result.error().GetMessage();
	
	Game game{ std::move(render_context_result).value() };
	dh::RenderExtent render_extent = game.GetRenderContext().GetSwapChainExtent();
	game.OnWindowResized(render_extent.width, render_extent.height);

	auto last_update_time = std::chrono::steady_clock::now();
	bool swap_chain_needs_recreation = false;
	std::optional<std::string> fatal_error;

	while (!s_token.stop_requested())
	{
		dh::WindowSize const window_size = window_size_pixels.load();
		if (swap_chain_needs_recreation || window_size != last_window_size)
		{
			last_window_size = window_size;
			auto recreate_result = game.GetRenderContext().RecreateSwapChain(window_size.width, window_size.height);
			if (!recreate_result)
			{
				fatal_error = recreate_result.error().GetMessage();
				break;
			}

			render_extent = game.GetRenderContext().GetSwapChainExtent();
			swap_chain_needs_recreation = render_extent.width <= 0 || render_extent.height <= 0;
			if (!swap_chain_needs_recreation)
				game.OnWindowResized(render_extent.width, render_extent.height);
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
		dt = std::clamp(dt, 0.0f, 0.05f);
		last_update_time = cur_time;

		input.NewFrame();
		game.Update(dt, input);

		dh::DrawFrameResult draw_result = window.DrawFrame(
			game.GetRenderContext(),
			[&game]() { game.Render(); });

		if (draw_result == dh::DrawFrameResult::SurfaceLost)
			break; // The Cosmic compositor has issues

		if (draw_result == dh::DrawFrameResult::SwapChainOutOfDate)
			swap_chain_needs_recreation = true;

		if (!game.ApplyPendingTransition())
			break;
	}

	game.GetRenderContext().WaitForLastFrame();
	return fatal_error;
}

std::optional<std::string> run_update_render_loop(
	dh::Window const & window,
	Input & input,
	std::atomic<dh::WindowSize> const & window_size_pixels,
	std::stop_token const & s_token)
{
	std::optional<std::string> fatal_error;
	try
	{
		fatal_error = run_update_render_loop_impl(window, input, window_size_pixels, s_token);
	}
	catch (std::exception const & err)
	{
		fatal_error = "Unexpected error in the update/render loop: " + std::string{ err.what() };
	}
	catch (...)
	{
		fatal_error = "Unknown error in the update/render loop.";
	}

	if (fatal_error)
		on_error(*fatal_error);
	window.SetShouldClose(true); // signal main thread to exit
	window.WakeEventLoop();
	return fatal_error;
}

std::optional<std::string> run_application()
{
	LOG(INFO) << "Initializing " << FullTitle << " version " << Version << "...";

	dh::Window window(dh::WindowSize{ 1920, 1080 }, FullTitle, on_error);
	if (!window.IsValid())
		return "Failed to initialize the application window. See the log for details.";
	window.ToggleFullscreen(); // start fullscreen

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
	window.SetOnFocusChanged([&input](bool focused)
		{
			if (!focused)
				input.Clear();
		});

	LOG(INFO) << "Running app...";

	std::optional<std::string> fatal_error;
	std::jthread update_render_loop(
		[&window, &input, &window_size_pixels, &fatal_error](std::stop_token s_token)
		{
			fatal_error = run_update_render_loop(window, input, window_size_pixels, s_token);
		});

	while (!window.ShouldClose())
		window.WaitEvents(1.0 / 120.0); // wakes immediately for input and window events

	update_render_loop.request_stop();
	if (update_render_loop.joinable())
		update_render_loop.join();

	return fatal_error;
}

int main(int argc, char * argv[])
{
	LoggingLifetime logging{ argc > 0 && argv[0] ? argv[0] : FullTitle };

	std::optional<std::string> fatal_error = run_application();
	if (fatal_error)
	{
		PlatformUtils::ShowErrorDialog(FullTitle, *fatal_error);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
