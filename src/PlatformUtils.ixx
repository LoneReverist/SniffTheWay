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
		std::wstring wide_message = Utf8ToUtf16(message);
		if (wide_title.empty() && !title.empty())
			wide_title = L"Sniff the Way";
		if (wide_message.empty() && !message.empty())
			wide_message = L"An unrecoverable error occurred. See the log for details.";

		MessageBoxW(
			nullptr,
			wide_message.c_str(),
			wide_title.c_str(),
			MB_OK | MB_ICONERROR | MB_TASKMODAL);
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
