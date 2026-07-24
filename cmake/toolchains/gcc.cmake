# ——————————————————————————————————————————————————————————
# GCC toolchain — use for Linux builds where Clang hits a
# compiler-specific bug (e.g. openal-soft's optional::emplace()
# issue under Clang: https://github.com/kcat/openal-soft/issues/1281).
#
# Linux:
#   cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/gcc.cmake -B build/linux ..
#
# This file's only job is to point CMake at gcc/g++ so that the
# correct (non-MSVC) branch in cmake/compiler_flags.cmake is used.
# ——————————————————————————————————————————————————————————

if(_TOOLCHAIN_GCC_INCLUDED)
    return()
endif()
set(_TOOLCHAIN_GCC_INCLUDED TRUE)

set(CMAKE_C_COMPILER   gcc CACHE STRING "C compiler"   FORCE)
set(CMAKE_CXX_COMPILER g++ CACHE STRING "C++ compiler" FORCE)