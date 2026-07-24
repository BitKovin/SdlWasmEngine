#!/usr/bin/env bash
set -euo pipefail

export CMAKE_BUILD_PARALLEL_LEVEL=18
echo '=== Configuring [linux] (sniper container) ==='
cmake -S "/repo" -B "/repo/build/linux" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="/repo/cmake/toolchains/clang.cmake" -DGAME_NAME="ShooterGame" 

echo '=== Building [linux] ==='
cmake --build "/repo/build/linux" --config Release
