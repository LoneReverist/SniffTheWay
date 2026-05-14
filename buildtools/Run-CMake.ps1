Push-Location $PSScriptRoot/..

New-Item -ItemType Directory -Force -Path "build-release"
cd build-release
cmake -B . -S .. --preset "x64-windows-static" -DCMAKE_BUILD_TYPE=Release
cmake --build . --target SniffTheWay

Pop-Location

# Linux
# mkdir build-release
# cd build-release
# cmake -B . -S .. --preset "x64-linux" -DCMAKE_C_COMPILER=clang-19 -DCMAKE_CXX_COMPILER=clang++-19 -DCMAKE_CXX_COMPILER_CLANG_SCAN_DEPS=/usr/lib/llvm-19/bin/clang-scan-deps -DCMAKE_CXX_COMPILER_CLANG_RESOURCE_DIR=/usr/lib/llvm-19/lib/clang/19 -DCMAKE_BUILD_TYPE=Release
# cmake --build . --target SniffTheWay
