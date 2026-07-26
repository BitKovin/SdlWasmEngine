cmake_minimum_required(VERSION 3.22)
project(AndroidEntry LANGUAGES C CXX)

# ——————————————————————————————————————————————————————————
# Android entry point, invoked by Gradle's externalNativeBuild
# (see ../build.gradle) with -DGAME_NAME=... forwarded from there.
#
# The official SDL2 android-project template builds SDL and your game as
# two separate add_subdirectory() calls from a file at this same path
# (jni/CMakeLists.txt -> add_subdirectory(SDL); add_subdirectory(src)).
# This engine already owns that whole graph — SDL, every other third-party
# library, the "engine" static lib, and the game target — in the real
# CMakeLists.txt three directories up. Re-declaring any of it here would
# just create a second, divergent copy of the build, so this file does the
# minimum instead: it tells that CMakeLists.txt where the Android app
# module's assets/ directory is (so GameData lands somewhere Gradle will
# actually package — see BuildScripts/PrepareGameData.cmake), then hands
# off to it. cmake/platforms/android.cmake takes it from there, exactly
# like cmake/platforms/linux.cmake does for a desktop build.
# ——————————————————————————————————————————————————————————

get_filename_component(ENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

get_filename_component(ANDROID_APP_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src/main/assets" ABSOLUTE)
set(ANDROID_APP_ASSETS_DIR "${ANDROID_APP_ASSETS_DIR}" CACHE PATH
    "Android module's assets/ dir — GameData is copied here so Gradle packages it into the APK" FORCE)

add_subdirectory("${ENGINE_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/engine")
