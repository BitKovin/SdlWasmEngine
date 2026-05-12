# cmake/toolchains/clang.cmake
# ——————————————————————————————————————————————————————————
# Clang toolchain — use for Linux and Emscripten builds.
#
# Linux:
#   cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang.cmake -B build/linux ..
#
# Emscripten:
#   'emsdk activate' / emcmake already sets the compiler to emcc/em++.
#   This file is not required for Emscripten, but passing it is harmless.
#
# This file's only job is to point CMake at clang/clang++ so that the
# correct (non-MSVC) branch in cmake/compiler_flags.cmake is used.
# ——————————————————————————————————————————————————————————

if(_TOOLCHAIN_CLANG_INCLUDED)
    return()
endif()
set(_TOOLCHAIN_CLANG_INCLUDED TRUE)

set(CMAKE_C_COMPILER   clang   CACHE STRING "C compiler"   FORCE)
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "C++ compiler" FORCE)
