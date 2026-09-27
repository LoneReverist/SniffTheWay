# Windows Unicode path regression

From the repository root, using the same MSVC and vcpkg installation as the game:

```powershell
cmake -S tests/unicode-paths -B .codex-temp/unicode-path-tests -G "Visual Studio 18 2026" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build .codex-temp/unicode-path-tests --config Release
./tests/unicode-paths/Run.ps1
```

The test compiles the game's actual PlatformUtils and StbImage modules. It runs
from directories containing Japanese characters, an accented letter and an emoji,
including an extended Windows path longer than 260 characters. It checks executable
discovery, Unicode image filenames, RGB/RGBA output, vertical flipping, and missing
and corrupt images. No graphics context or game window is needed.

Fixtures are retained under the test build directory's `runs` folder. Long-path
testing uses extended path syntax; it does not enable Windows long-path policy or
claim that every game dependency supports long paths.
