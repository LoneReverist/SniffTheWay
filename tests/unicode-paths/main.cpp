#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>
#define NOMINMAX
#include <windows.h>
#undef LoadImage

import PlatformUtils;
import StbImage;

void require(bool condition, char const * message)
{
    if (!condition) throw std::runtime_error(message);
}

int wmain(int argc, wchar_t ** argv)
{
    try
    {
        if (argc == 3 && std::wstring_view(argv[1]) == L"--launch-long")
        {
            // An explicit application name avoids the MAX_PATH limit on the
            // executable token in CreateProcess's command-line-only form.
            std::wstring command = L"\"" + std::wstring(argv[2]) + L"\" --long";
            STARTUPINFOW startup{};
            startup.cb = sizeof(startup);
            PROCESS_INFORMATION process{};
            require(CreateProcessW(argv[2], command.data(), nullptr, nullptr, FALSE,
                0, nullptr, nullptr, &startup, &process), "Could not launch long-path test.");
            CloseHandle(process.hThread);
            WaitForSingleObject(process.hProcess, INFINITE);
            DWORD result = 1;
            GetExitCodeProcess(process.hProcess, &result);
            CloseHandle(process.hProcess);
            return static_cast<int>(result);
        }
        auto const executable = PlatformUtils::GetExecutablePath();
        auto const directory = PlatformUtils::GetExecutableDir();
        require(std::filesystem::exists(executable), "Executable path does not resolve.");
        require(executable.filename() == L"UnicodePathTests.exe", "Incorrect executable filename.");
        require(directory.native().find(L"\u65e5\u672c\u8a9e-\u00e9-\U0001f43e") != std::wstring::npos,
            "Unicode directory was lost.");
        if (argc > 1 && std::wstring_view(argv[1]) == L"--long")
            require(executable.native().size() > 260, "Long-path case did not exceed MAX_PATH.");

        auto const image_path = directory / L"\u753b\u50cf-\U0001f43e.ppm";
        {
            std::ofstream file(image_path, std::ios::binary);
            file << "P6\n1 2\n255\n";
            unsigned char const pixels[] = {255, 0, 0, 0, 0, 255};
            file.write(reinterpret_cast<char const *>(pixels), sizeof(pixels));
            require(file.good(), "Could not create test image.");
        }
        StbImage image(image_path, 4);
        require(image.IsValid() && image.GetWidth() == 1 && image.GetHeight() == 2, "Unicode image load failed.");
        require(image.GetData()[0] == 255 && image.GetData()[3] == 255, "Incorrect RGBA pixels.");
        image.LoadImage(image_path, 3, true);
        require(image.IsValid() && image.GetData()[2] == 255, "Vertical flip or RGB load failed.");
        image.LoadImage(directory / L"missing.png", 4);
        require(!image.IsValid() && image.GetWidth() == 0, "Missing image retained old data.");
        {
            std::ofstream file(image_path, std::ios::binary | std::ios::trunc);
            file << "invalid image";
        }
        image.LoadImage(image_path, 4);
        require(!image.IsValid(), "Corrupt image was accepted.");
        std::filesystem::remove(image_path);
        std::cout << "PASS: native executable path, Unicode image, flip, channels, missing and corrupt files.\n";
        return 0;
    }
    catch (std::exception const & error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
