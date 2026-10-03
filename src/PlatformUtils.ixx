// PlatformUtils.ixx

module;

#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>

#endif

export module PlatformUtils;

namespace PlatformUtils
{
#if defined(_WIN32)
	std::wstring ToWindowsLineEndings(std::wstring_view text)
	{
		std::wstring result;
		result.reserve(text.size());
		for (std::size_t i = 0; i < text.size(); ++i)
		{
			if (text[i] == L'\n' && (i == 0 || text[i - 1] != L'\r'))
				result.push_back(L'\r');
			result.push_back(text[i]);
		}
		return result;
	}

	LRESULT CALLBACK ErrorDialogProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
	{
		if (message == WM_CREATE)
		{
			auto const * create = reinterpret_cast<CREATESTRUCTW const *>(l_param);
			auto const * text = static_cast<std::wstring const *>(create->lpCreateParams);
			HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

			HWND edit = CreateWindowExW(
				WS_EX_CLIENTEDGE, L"EDIT", text->c_str(),
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
				16, 16, 488, 230, window, reinterpret_cast<HMENU>(1001), GetModuleHandleW(nullptr), nullptr);
			HWND copy = CreateWindowExW(
				0, L"BUTTON", L"Copy",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				336, 260, 80, 28, window, reinterpret_cast<HMENU>(1002), GetModuleHandleW(nullptr), nullptr);
			HWND ok = CreateWindowExW(
				0, L"BUTTON", L"OK",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				424, 260, 80, 28, window, reinterpret_cast<HMENU>(IDOK), GetModuleHandleW(nullptr), nullptr);
			if (!edit || !copy || !ok)
				return -1;
			SendMessageW(edit, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
			SendMessageW(copy, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
			SendMessageW(ok, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
			SetFocus(edit);
			return 0;
		}
		if (message == WM_COMMAND)
		{
			if (LOWORD(w_param) == IDOK)
			{
				DestroyWindow(window);
				return 0;
			}
			if (LOWORD(w_param) == 1002)
			{
				HWND edit = GetDlgItem(window, 1001);
				int const length = GetWindowTextLengthW(edit);
				if (OpenClipboard(window))
				{
					EmptyClipboard();
					std::size_t const size = (static_cast<std::size_t>(length) + 1) * sizeof(wchar_t);
					HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, size);
					if (memory)
					{
						void * destination = GlobalLock(memory);
						if (destination)
						{
							GetWindowTextW(edit, static_cast<wchar_t *>(destination), length + 1);
							GlobalUnlock(memory);
							if (!SetClipboardData(CF_UNICODETEXT, memory)) GlobalFree(memory);
						}
						else GlobalFree(memory);
					}
					CloseClipboard();
				}
				return 0;
			}
		}
		if (message == WM_CLOSE)
		{
			DestroyWindow(window);
			return 0;
		}
		return DefWindowProcW(window, message, w_param, l_param);
	}

	std::wstring Utf8ToUtf16(std::string_view text)
	{
		if (text.empty())
			return {};
		if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
			return {};

		int const text_size = static_cast<int>(text.size());
		int wide_size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), text_size, nullptr, 0);
		DWORD conversion_flags = MB_ERR_INVALID_CHARS;
		if (wide_size <= 0)
		{
			// Preserve as much diagnostic text as possible if a driver supplied malformed UTF-8.
			conversion_flags = 0;
			wide_size = MultiByteToWideChar(CP_UTF8, conversion_flags, text.data(), text_size, nullptr, 0);
		}
		if (wide_size <= 0)
			return {};

		std::wstring result(static_cast<std::size_t>(wide_size), L'\0');
		if (MultiByteToWideChar(CP_UTF8, conversion_flags, text.data(), text_size, result.data(), wide_size) <= 0)
			return {};
		return result;
	}
#endif

	export void ShowErrorDialog(std::string_view title, std::string_view message)
	{
#if defined(_WIN32)
		std::wstring wide_title = Utf8ToUtf16(title);
		std::wstring wide_message = ToWindowsLineEndings(Utf8ToUtf16(message));
		if (wide_title.empty() && !title.empty())
			wide_title = L"Sniff the Way";
		if (wide_message.empty() && !message.empty())
			wide_message = L"An unrecoverable error occurred. See the log for details.";

		static wchar_t const class_name[] = L"SniffTheWayErrorDialog";
		static bool registered = false;
		if (!registered)
		{
			WNDCLASSEXW window_class{ sizeof(WNDCLASSEXW) };
			window_class.lpfnWndProc = ErrorDialogProc;
			window_class.hInstance = GetModuleHandleW(nullptr);
			window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW
			window_class.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(32513)); // IDI_ERROR
			window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
			window_class.lpszClassName = class_name;
			if (!RegisterClassExW(&window_class) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
			{
				MessageBoxW(nullptr, wide_message.c_str(), wide_title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
				return;
			}
			registered = true;
		}

		HWND dialog = CreateWindowExW(
			WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
			class_name,
			wide_title.c_str(),
			WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, 536, 336,
			nullptr, nullptr, GetModuleHandleW(nullptr), &wide_message);
		if (!dialog)
		{
			MessageBoxW(nullptr, wide_message.c_str(), wide_title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
			return;
		}
		RECT rect{};
		GetWindowRect(dialog, &rect);
		int const x = (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
		int const y = (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
		SetWindowPos(dialog, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		MSG msg{};
		while (IsWindow(dialog) && GetMessageW(&msg, nullptr, 0, 0) > 0)
		{
			if (!IsDialogMessageW(dialog, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
#else
		// Linux has no universal native dialog API without a desktop-toolkit dependency.
		std::cerr << title << "\n\n" << message << '\n';
#endif
	}

	export std::filesystem::path GetExecutablePath()
	{
		std::filesystem::path path;

#if defined(_WIN32)
		// Windows paths are UTF-16. A successful call can still return a
		// truncated name, so grow the buffer until the complete path fits.
		std::wstring buffer(MAX_PATH, L'\0');
		for (;;)
		{
			DWORD const count = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
			if (count == 0)
				throw std::runtime_error("Failed to get executable path.");
			if (count < buffer.size())
			{
				buffer.resize(count);
				path = std::filesystem::path(buffer);
				break;
			}
			if (buffer.size() >= 32768)
				throw std::runtime_error("Executable path exceeds the Windows path limit.");
			buffer.resize(buffer.size() > 16384 ? 32768 : buffer.size() * 2);
		}

#elif defined(__linux__)
		char buffer[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
		if (count != -1)
		{
			buffer[count] = '\0';
			path = std::filesystem::path(buffer);
		}

#else
		static_assert(false, "Unsupported platform.");
#endif

		if (path.empty())
		{
			throw std::runtime_error("Failed to get executable path.");
		}
		return path;
	}

	export std::filesystem::path GetExecutableDir()
	{
		return GetExecutablePath().parent_path();
	}
}
