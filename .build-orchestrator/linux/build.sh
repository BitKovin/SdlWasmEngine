#!/usr/bin/env bash
set -euo pipefail

export CMAKE_BUILD_PARALLEL_LEVEL=18
echo '=== Configuring [linux] ==='
cmake -S "/mnt/f/Git/SdlWasmEngine" -B "/mnt/f/Git/SdlWasmEngine/build/linux" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="/mnt/f/Git/SdlWasmEngine/cmake/toolchains/clang.cmake" -DGAME_NAME="ShooterGame" 

echo '=== Building [linux] ==='
cmake --build "/mnt/f/Git/SdlWasmEngine/build/linux" --config Release
