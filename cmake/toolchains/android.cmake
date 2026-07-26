# cmake/toolchains/android.cmake
# ——————————————————————————————————————————————————————————
# Android toolchain — thin wrapper around the NDK's own toolchain file.
#
# Unlike the other toolchains/*.cmake files in this project, Android's real
# toolchain logic (sysroot, per-ABI flags, clang target triples, API-level
# gating) is intentionally NOT reimplemented here — the NDK ships a
# well-tested android.toolchain.cmake that Gradle's externalNativeBuild also
# delegates to under the hood, and hand-rolling that logic tends to drift
# out of sync with new NDK releases. This file just locates the NDK and
# hands off to it, the same way cmake/toolchains/clang.cmake just points at
# clang/clang++ so cmake/compiler_flags.cmake takes the right branch.
#
# Command-line build (mirrors the desktop invocations documented at the top
# of the root CMakeLists.txt):
#   cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/android.cmake ^
#         -DANDROID_ABI=arm64-v8a -DGAME_NAME=MyGame -B build/android ..
#
# In practice you'll usually reach this indirectly through Gradle instead —
# see android/app/build.gradle's externalNativeBuild block, which is what
# most Android builds (and Android Studio) will actually invoke. Gradle
# passes ANDROID_ABI / ANDROID_PLATFORM per-variant automatically; this file
# only supplies defaults for the rare case of configuring CMake directly.
# ——————————————————————————————————————————————————————————

if(_TOOLCHAIN_ANDROID_INCLUDED)
    return()
endif()
set(_TOOLCHAIN_ANDROID_INCLUDED TRUE)

# ---- Locate the NDK ----
# Same lookup order Gradle/Android Studio use: an explicit -DANDROID_NDK_HOME,
# then the ANDROID_NDK_HOME env var, then ANDROID_NDK_ROOT (also commonly set).
if(DEFINED ANDROID_NDK_HOME)
    set(_NDK_ROOT "${ANDROID_NDK_HOME}")
elseif(DEFINED ENV{ANDROID_NDK_HOME})
    set(_NDK_ROOT "$ENV{ANDROID_NDK_HOME}")
elseif(DEFINED ENV{ANDROID_NDK_ROOT})
    set(_NDK_ROOT "$ENV{ANDROID_NDK_ROOT}")
else()
    message(FATAL_ERROR
        "Android builds need the NDK. Set ANDROID_NDK_HOME (env var, or pass "
        "-DANDROID_NDK_HOME=/path/to/ndk/28.2.13676358 on the configure line) "
        "to a side-by-side NDK install, e.g. .../Android/Sdk/ndk/28.2.13676358")
endif()

file(TO_CMAKE_PATH "${_NDK_ROOT}" _NDK_ROOT)

if(NOT EXISTS "${_NDK_ROOT}/build/cmake/android.toolchain.cmake")
    message(FATAL_ERROR "No NDK toolchain file found under: ${_NDK_ROOT}")
endif()

message(STATUS "Android NDK: ${_NDK_ROOT}")

# ---- Defaults for a direct `cmake` invocation ----
# All of these are the exact cache variables the NDK toolchain file reads;
# Gradle sets ANDROID_ABI / ANDROID_PLATFORM itself per build variant (see
# android/app/build.gradle), so these defaults only kick in when nothing
# upstream already provided a value.
if(NOT DEFINED ANDROID_ABI)
    set(ANDROID_ABI "arm64-v8a" CACHE STRING "Target Android ABI")
endif()

if(NOT DEFINED ANDROID_PLATFORM)
    # Keep this in sync with app/build.gradle's minSdkVersion.
    set(ANDROID_PLATFORM "android-24" CACHE STRING "Minimum Android API level")
endif()

if(NOT DEFINED ANDROID_STL)
    # c++_shared, not c++_static: FMOD ships its own prebuilt .so on
    # Android, so more than one native library is loaded into the process.
    # Sharing one copy of the C++ runtime across them avoids the ODR /
    # duplicate-global-state problems that come from each .so statically
    # linking its own copy. If you ever confirm FMOD is the only extra .so
    # and would rather not ship libc++_shared.so separately, c++_static is
    # the alternative — just verify against every .so you actually load.
    set(ANDROID_STL "c++_shared" CACHE STRING "C++ STL flavor")
endif()

# ---- Hand off to the NDK ----
include("${_NDK_ROOT}/build/cmake/android.toolchain.cmake")
