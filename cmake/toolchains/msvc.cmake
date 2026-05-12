# cmake/toolchains/msvc.cmake
# ——————————————————————————————————————————————————————————
# MSVC toolchain — use for Windows and Xbox GDK builds.
#
# Windows:
#   cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake -B build/windows ..
#
# Xbox GDK:
#   Activate the GDK environment first (GameDK env vars / VS prompt),
#   then pass this same toolchain file — the platform file handles the rest.
#
# Run cmake from a Visual Studio Developer Command Prompt (or call
# vcvarsall.bat x64 first) so cl.exe and link.exe are on PATH.
#
# This file's jobs:
#   1. Set CMAKE_MSVC_RUNTIME_LIBRARY before project() — the only safe
#      place to do so (toolchain files run before project()).
#   2. Confirm cl.exe as the compiler so cmake/compiler_flags.cmake
#      takes the MSVC branch and emits /O2 /Zi instead of -O2 -g.
# ——————————————————————————————————————————————————————————

if(_TOOLCHAIN_MSVC_INCLUDED)
    return()
endif()
set(_TOOLCHAIN_MSVC_INCLUDED TRUE)

# Static CRT: MT in Release, MTd in Debug.
# Must be set before project() — hence it lives here, not in compiler_flags.cmake.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    CACHE STRING "MSVC runtime library" FORCE)

# cl.exe is found automatically when vcvarsall has been sourced.
# Naming it explicitly prevents CMake from ever falling back to gcc/clang
# if both happen to be on PATH.
set(CMAKE_C_COMPILER   cl CACHE STRING "C compiler"   FORCE)
set(CMAKE_CXX_COMPILER cl CACHE STRING "C++ compiler" FORCE)
