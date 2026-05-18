cmake_minimum_required(VERSION 3.16)

# ——————————————————————————————————————————————————————————
# ENGINE_ROOT — always points to this file's directory,
# regardless of whether this is the top-level project or
# consumed via add_subdirectory() from another project.
# Every path in this file uses ENGINE_ROOT, never
# CMAKE_SOURCE_DIR (which would point to the consumer's root).
# ——————————————————————————————————————————————————————————
set(ENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# ——————————————————————————————————————————————————————————
# Toolchain / platform pairing (pass via -DCMAKE_TOOLCHAIN_FILE=…)
#
#  Platform        Toolchain file
#  ──────────────  ─────────────────────────────────────
#  Linux           cmake/toolchains/clang.cmake
#  Emscripten      cmake/toolchains/clang.cmake  (or skip — emcmake sets the compiler itself)
#  Windows         cmake/toolchains/msvc.cmake
#  Xbox GDK        cmake/toolchains/msvc.cmake   (activate GDK env first)
#
# All targets build with Ninja:
#   cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/<clang|msvc>.cmake ..
#
# Windows / Xbox GDK: run from a VS Developer Command Prompt so cl.exe is on PATH.
# ——————————————————————————————————————————————————————————

# ——————————————————————————————————————————————————————————
# Platform detection — includes the matching platform file which
# defines the four hook functions used throughout this file.
# PLATFORM_DIR and PLATFORM_HAS_OPENAL are also set here.
# ——————————————————————————————————————————————————————————

if(NOT ENGINE_DISABLE_PLATFORM_DETECTION)


    if(CMAKE_SYSTEM_NAME MATCHES "Emscripten" OR DEFINED EMSCRIPTEN)
        include("${ENGINE_ROOT}/cmake/platforms/emscripten.cmake")

    # 1. Check for EXPLICIT Windows GDK flag first
    elseif(DEFINED WINDOWS_GDK)
        include("${ENGINE_ROOT}/cmake/platforms/windows_gdk.cmake")
    # 2. Then check for Xbox GDK (Manual or Detected)
    elseif(DEFINED XBOX_GDK OR CMAKE_SYSTEM_NAME MATCHES "Durango|Scarlett|GXDKDesktop")
        include("${ENGINE_ROOT}/cmake/platforms/xbox_gdk.cmake")
    # 3. Fallback to standard Windows
    elseif(WIN32 OR MSVC)
        include("${ENGINE_ROOT}/cmake/platforms/windows.cmake")

    elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
        include("${ENGINE_ROOT}/cmake/platforms/linux.cmake")
    else()
        message(WARNING "No platform file matched '${CMAKE_SYSTEM_NAME}'. Using no-op stubs.")
        set(PLATFORM_DIR "unknown")
        function(platform_early_config)
        endfunction()
        function(platform_pre_libraries)
        endfunction()
        function(platform_post_libraries t)
        endfunction()
        function(platform_configure_game_target t)
        endfunction()
    endif()

else()

    message("Platform auto-detection is disabled. Using no-op stubs.")
    set(PLATFORM_DIR "unknown")
    function(platform_early_config)
    endfunction()
    function(platform_pre_libraries)
    endfunction()
    function(platform_post_libraries t)
    endfunction()
    function(platform_configure_game_target t)
    endfunction()
    

endif()


project(MyProject)

message(STATUS "==================================================")
message(STATUS "Platform : ${PLATFORM_DIR}")
message(STATUS "==================================================")

# ——————————————————————————————————————————————————————————
# Library mode — when ON the game executable block is skipped
# entirely and only the engine static library is produced.
# The consuming project sets this before add_subdirectory().
# ——————————————————————————————————————————————————————————
option(ENGINE_AS_LIBRARY "Skip game executable; expose only the engine target" OFF)

# ——————————————————————————————————————————————————————————
# Game selection (user-visible, cached) — ignored in library mode
# ——————————————————————————————————————————————————————————
# Pass -DGAME_NAME=MyGame  or  -DGAME_PATH=/absolute/path
set(GAME_NAME "Game" CACHE STRING
    "Name of the game project subdirectory under source/Games")
set(GAME_PATH "" CACHE PATH
    "Optional path to the game project directory. Overrides GAME_NAME.")

# ——————————————————————————————————————————————————————————
# Global settings
# ——————————————————————————————————————————————————————————
option(ENABLE_PTHREADS "Enable Emscripten pthread support (WASM threads)" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force static builds for all vendored libraries on all platforms.
# Only applied when we are the top-level project; when consumed as a
# library the consumer controls this for their own targets.
if(NOT ENGINE_AS_LIBRARY)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
endif()

# ——————————————————————————————————————————————————————————
# Compiler-agnostic optimisation / debug flags
# Sets the equivalents of -O2 -DNDEBUG -g for every compiler
# (Clang, GCC, MSVC).  Must come after project() so the compiler
# is known, and before any target is created.
# ——————————————————————————————————————————————————————————

if(NOT ENGINE_DISABLE_COMPILER_FLAGS)
    include("${ENGINE_ROOT}/cmake/compiler_flags.cmake")
else()
    message("Global compiler flags are disabled. Make sure to set optimisation and debug flags manually.")
endif()

# ——————————————————————————————————————————————————————————
# Dynamic CRT override — /MD instead of /MT.
# Must be set by the consumer BEFORE add_subdirectory() on
# the engine.  The Qt editor sets this ON because Qt is
# always built against the dynamic CRT.
# Has no effect on non-MSVC toolchains or standalone builds.
# ——————————————————————————————————————————————————————————
option(ENGINE_USE_DYNAMIC_RUNTIME
    "Use /MD (dynamic CRT) for MSVC. Required when linking with Qt." OFF)

if(MSVC AND ENGINE_USE_DYNAMIC_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
        CACHE STRING "MSVC runtime library" FORCE)
    foreach(_lang C CXX)
        foreach(_cfg "" _DEBUG _RELEASE _MINSIZEREL _RELWITHDEBINFO)
            set(_var "CMAKE_${_lang}_FLAGS${_cfg}")
            string(REPLACE "/MTd" "/MDd" ${_var} "${${_var}}")
            string(REPLACE "/MT"  "/MD"  ${_var} "${${_var}}")
            set(${_var} "${${_var}}" CACHE STRING "" FORCE)
        endforeach()
    endforeach()
    # Jolt explicitly controls its own CRT via this option — must override it too.
    set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
    message(STATUS "ENGINE_USE_DYNAMIC_RUNTIME ON — using /MD (dynamic CRT)")
endif()

# ——————————————————————————————————————————————————————————
# Hook 1 — early platform compile config
# (compiler flags, sysroot includes, pthread compile option …)
# ——————————————————————————————————————————————————————————
platform_early_config()

# ——————————————————————————————————————————————————————————
# Engine static library
# ——————————————————————————————————————————————————————————
file(GLOB_RECURSE ENGINE_SOURCES
    "${ENGINE_ROOT}/source/Engine/*.cpp"
    "${ENGINE_ROOT}/source/Engine/*.c"
)

add_library(engine STATIC ${ENGINE_SOURCES})

# Standard C++ exception handling — applied to our targets only, not globally,
# to avoid D9025 warnings from vendored libs that disable exceptions themselves.
if(MSVC)
    target_compile_options(engine PRIVATE /EHsc)
endif()

target_include_directories(engine
    PUBLIC
        "${ENGINE_ROOT}/source/Engine"   # <Renderer/…>
        "${ENGINE_ROOT}/libraries"       # third-party headers
)

set(LIB_DIR "${ENGINE_ROOT}/sourceLibraries")

# ——————————————————————————————————————————————————————————
# Hook 2 — platform-specific CMake cache vars for third-party libs
# (must run before the first add_subdirectory below)
# ——————————————————————————————————————————————————————————
platform_pre_libraries()

# ——————————————————————————————————————————————————————————
# Third-party libraries
# ——————————————————————————————————————————————————————————

# --- OpenAL-Soft ---
# Skipped on platforms where PLATFORM_HAS_OPENAL is FALSE
if(PLATFORM_HAS_OPENAL)

    set(LIBTYPE STATIC CACHE STRING "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)

    add_subdirectory(${LIB_DIR}/openAlSoft)
    target_link_libraries(engine PUBLIC OpenAL)
endif()

# --- RmlUi (vendored) ---
set(RMLUI_BUILD_SAMPLES   OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_LUA       OFF CACHE BOOL "" FORCE)
set(RMLUI_ENABLE_DEBUGGER ON  CACHE BOOL "" FORCE)

add_subdirectory(${LIB_DIR}/RmlUi-6.1/Dependencies/freetype)
add_library(Freetype::Freetype ALIAS freetype)
target_link_libraries(engine PUBLIC Freetype::Freetype)

add_subdirectory("${LIB_DIR}/RmlUi-6.1")
target_link_libraries(engine PUBLIC RmlUi::RmlUi RmlUi::Debugger)

if(PLATFORM_CUSTOM_SDL)

    message("using custom SDL for this platform")

else()

    # --- SDL2 (vendored) ---
    add_subdirectory("${LIB_DIR}/SDL")
    target_link_libraries(engine PUBLIC SDL2::SDL2)

endif()



# --- FMOD ---
add_subdirectory(${LIB_DIR}/fmod)
target_link_libraries(engine PUBLIC fmod)

# --- JoltPhysics ---
# Shared Jolt shipping configuration (platform file may have set USE_WASM_SIMD)
set(ENABLE_ASSERTS               OFF CACHE BOOL "" FORCE)
set(JPH_NO_DEBUG                 ON  CACHE BOOL "" FORCE)
set(ENABLE_ALL_WARNINGS          OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION ON  CACHE BOOL "" FORCE)
set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE OFF CACHE BOOL "" FORCE)
set(PROFILER_IN_DEBUG_AND_RELEASE       OFF CACHE BOOL "" FORCE)

add_subdirectory(${LIB_DIR}/JoltPhysics-5.3.0/Build/)
target_link_libraries(engine PUBLIC Jolt::Jolt)

# --- Assimp ---
set(ASSIMP_BUILD_TESTS            OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS     OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES          OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT              ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DLL_EXPORT       OFF)
set(ASSIMP_INSTALL                OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER     ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER    ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB             ON  CACHE BOOL "" FORCE)

add_subdirectory(${LIB_DIR}/assimp)
target_link_libraries(engine PUBLIC assimp)

# --- BGFX ---
# Shared BGFX knobs (platform file sets BGFX_BUILD_TOOLS and threading flags)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL        OFF CACHE BOOL "" FORCE)

add_subdirectory(${LIB_DIR}/bgfx/bgfx.cmake)
target_link_libraries(engine PUBLIC bgfx bx)

# --- bit7z ---
set(BIT7Z_AUTO_FORMAT ON CACHE BOOL "enable auto format support" FORCE)
add_subdirectory(${LIB_DIR}/bit7z)
target_link_libraries(engine PUBLIC bit7z)

# ——————————————————————————————————————————————————————————
# Hook 3 — post-library platform fixups
# (e.g. bimg_encode SSE workaround on Emscripten)
# ——————————————————————————————————————————————————————————
platform_post_libraries(engine)

if(ENGINE_AS_LIBRARY)
    target_compile_definitions(engine PUBLIC ENGINE_AS_LIBRARY)
endif()

# ——————————————————————————————————————————————————————————
# Game executable — skipped entirely when ENGINE_AS_LIBRARY=ON
# ——————————————————————————————————————————————————————————
if(NOT ENGINE_AS_LIBRARY)

    # ——————————————————————————————————————————————————————————
    # Resolve GAME_DIR
    # ——————————————————————————————————————————————————————————
    if(GAME_PATH)
        set(GAME_DIR "${GAME_PATH}")
    else()
        set(GAME_DIR "${ENGINE_ROOT}/source/Games/${GAME_NAME}")

        if(NOT IS_DIRECTORY "${GAME_DIR}" AND IS_DIRECTORY "${ENGINE_ROOT}/source/Game")
            message(WARNING
                "Game directory '${GAME_DIR}' not found. "
                "Falling back to legacy 'source/Game'. "
                "Consider passing -DGAME_NAME or -DGAME_PATH.")
            set(GAME_DIR "${ENGINE_ROOT}/source/Game")
        endif()
    endif()

    message(STATUS "Game source directory: ${GAME_DIR}")

    # ——————————————————————————————————————————————————————————
    # Output directory layout:  Build/<GAME_NAME>/<PLATFORM_DIR>/
    # ——————————————————————————————————————————————————————————
    set(OUTPUT_BASE_DIR "${ENGINE_ROOT}/Build/${GAME_NAME}/${PLATFORM_DIR}")
    file(MAKE_DIRECTORY "${OUTPUT_BASE_DIR}")
    file(MAKE_DIRECTORY "${OUTPUT_BASE_DIR}/bin" "${OUTPUT_BASE_DIR}/lib" "${OUTPUT_BASE_DIR}/assets")

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/bin")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib")

    foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER "${cfg}" CFGU)
        set("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFGU}" "${OUTPUT_BASE_DIR}/bin")
        set("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFGU}" "${OUTPUT_BASE_DIR}/lib")
        set("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFGU}" "${OUTPUT_BASE_DIR}/lib")
    endforeach()

    # Redirect the already-created engine target to the new output dirs
    set_target_properties(engine PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/bin"
    )

    message(STATUS "GAME_NAME      : ${GAME_NAME}")
    message(STATUS "OUTPUT_BASE_DIR: ${OUTPUT_BASE_DIR}")

    # ——————————————————————————————————————————————————————————
    # Collect game sources
    # ——————————————————————————————————————————————————————————
    file(GLOB_RECURSE GAME_SOURCES "${GAME_DIR}/*.cpp")

    set(LEGACY_GAME_DIR "${ENGINE_ROOT}/source/Game")
    if(IS_DIRECTORY "${LEGACY_GAME_DIR}")
        message(STATUS "Legacy game directory found: ${LEGACY_GAME_DIR}")
        file(GLOB_RECURSE LEGACY_GAME_SOURCES "${LEGACY_GAME_DIR}/*.cpp")
        list(APPEND GAME_SOURCES ${LEGACY_GAME_SOURCES})
        list(REMOVE_DUPLICATES GAME_SOURCES)
    endif()

    if(NOT GAME_SOURCES)
        message(FATAL_ERROR "No C++ sources found in ${GAME_DIR}. Cannot build ${GAME_NAME}.")
    endif()

    # ——————————————————————————————————————————————————————————
    # Game executable target
    # ——————————————————————————————————————————————————————————
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" GAME_TARGET_NAME "${GAME_NAME}")

    add_executable(${GAME_TARGET_NAME} ${GAME_SOURCES})

    if(MSVC)
        target_compile_options(${GAME_TARGET_NAME} PRIVATE /EHsc)
    endif()

    include(${ENGINE_ROOT}/BuildScripts/PrepareGameData.cmake)

    target_include_directories(${GAME_TARGET_NAME}
        PUBLIC "${GAME_DIR}"
    )

    target_link_libraries(${GAME_TARGET_NAME} PRIVATE engine)

    set_target_properties(${GAME_TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib"
    )

    # ——————————————————————————————————————————————————————————
    # Hook 4 — platform-specific game target finalisation
    # (output name / suffix, linker flags, install rules …)
    # ——————————————————————————————————————————————————————————
    platform_configure_game_target(${GAME_TARGET_NAME})

    # Persist last selected game for tooling convenience
    file(WRITE "${CMAKE_BINARY_DIR}/last_game.txt" "${GAME_NAME}\n")
    message(STATUS "Last game written to: ${CMAKE_BINARY_DIR}/last_game.txt")

endif() # NOT ENGINE_AS_LIBRARY