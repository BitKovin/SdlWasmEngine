# RunGameDataFinalize.cmake
#
# Standalone entry point that finalizes GameData exactly once, independent of
# any platform build tree. Meant to run as the very first build step, before
# any per-platform `cmake --build`, producing the shared, once-finalized
# location that every platform then either preloads directly (Emscripten) or
# copies from (everyone else) — see PrepareGameData.cmake / SKIP_GAMEDATA_FINALIZE.
#
# Needs only `cmake` and a `python3`/`python` interpreter on PATH — no
# compiler, no configured build directory, no toolchain file.
#
# Usage:
#   cmake -DGAME_NAME=MyGame -P BuildScripts/RunGameDataFinalize.cmake
#   cmake -DGAME_PATH=/abs/path/to/game -P BuildScripts/RunGameDataFinalize.cmake
#
# GAME_DIR resolution intentionally mirrors CMakeLists.txt's own logic
# (GAME_PATH override, else source/Games/<GAME_NAME>, else legacy
# source/Game) — keep the two in sync if either ever changes.

cmake_minimum_required(VERSION 3.16)

# ENGINE_ROOT = directory containing the engine's top-level CMakeLists.txt,
# i.e. one level up from this script (BuildScripts/RunGameDataFinalize.cmake).
get_filename_component(ENGINE_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(NOT DEFINED GAME_NAME OR "${GAME_NAME}" STREQUAL "")
    set(GAME_NAME "Game")
endif()

if(DEFINED GAME_PATH AND NOT "${GAME_PATH}" STREQUAL "")
    set(GAME_DIR "${GAME_PATH}")
else()
    set(GAME_DIR "${ENGINE_ROOT}/source/Games/${GAME_NAME}")

    if(NOT IS_DIRECTORY "${GAME_DIR}" AND IS_DIRECTORY "${ENGINE_ROOT}/source/Game")
        message(WARNING
            "Game directory '${GAME_DIR}' not found. Falling back to legacy 'source/Game'.")
        set(GAME_DIR "${ENGINE_ROOT}/source/Game")
    endif()
endif()

set(GAMEDATA_SCRIPT "${ENGINE_ROOT}/BuildScripts/finalize_gamedata.py")
set(GAMEDATA_SOURCE "${GAME_DIR}/GameData")

# Shared location: Build/<GAME_NAME>/GameData — NOT nested under a platform
# subfolder, so every platform build can either preload it directly
# (Emscripten) or copy from it (everyone else).
set(GAMEDATA_SHARED_DIR "${ENGINE_ROOT}/Build/${GAME_NAME}/GameData")

if(NOT EXISTS "${GAMEDATA_SCRIPT}")
    message(FATAL_ERROR "finalize_gamedata.py not found: ${GAMEDATA_SCRIPT}")
endif()

if(NOT EXISTS "${GAMEDATA_SOURCE}")
    message(FATAL_ERROR "GameData source not found: ${GAMEDATA_SOURCE}")
endif()

find_program(GAMEDATA_PYTHON_EXECUTABLE NAMES python3 python)
if(NOT GAMEDATA_PYTHON_EXECUTABLE)
    message(FATAL_ERROR "No python3/python interpreter found on PATH.")
endif()

message(STATUS "Finalizing GameData once: ${GAMEDATA_SOURCE} -> ${GAMEDATA_SHARED_DIR}")

execute_process(
    COMMAND "${GAMEDATA_PYTHON_EXECUTABLE}" "${GAMEDATA_SCRIPT}" "${GAMEDATA_SOURCE}" "${GAMEDATA_SHARED_DIR}"
    WORKING_DIRECTORY "${ENGINE_ROOT}"
    RESULT_VARIABLE GAMEDATA_FINALIZE_RESULT
)

if(NOT GAMEDATA_FINALIZE_RESULT EQUAL 0)
    message(FATAL_ERROR "finalize_gamedata.py failed with exit code ${GAMEDATA_FINALIZE_RESULT}")
endif()

message(STATUS "GameData finalized: ${GAMEDATA_SHARED_DIR}")
