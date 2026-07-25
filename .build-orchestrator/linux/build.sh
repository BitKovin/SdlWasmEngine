#!/usr/bin/env bash
set -euo pipefail

CMAKE_VERSION=3.31.12
if [ ! -x /cache/cmake/bin/cmake ]; then
  echo 'Bootstrapping CMake '$CMAKE_VERSION' into cache volume (first run only)...'
  curl -fsSL "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz" -o /tmp/cmake.tar.gz
  mkdir -p /cache/cmake
  tar -xzf /tmp/cmake.tar.gz -C /cache/cmake --strip-components=1
fi
export PATH=/cache/cmake/bin:$PATH
cmake --version
export CMAKE_BUILD_PARALLEL_LEVEL=18
echo '=== Configuring [linux] (sniper container) ==='
cmake -S "/repo" -B "/repo/build/linux" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="/repo/cmake/toolchains/clang.cmake" -DGAME_NAME="ShooterGame" -DSKIP_GAMEDATA_FINALIZE=1

echo '=== Building [linux] ==='
cmake --build "/repo/build/linux" --config Release
