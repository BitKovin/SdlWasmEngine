cmake_minimum_required(VERSION 3.16)
project(MyProject)

# ——————————————————————————————————————————————————————————
# Game selection (user-visible, cached)
# ——————————————————————————————————————————————————————————
# You can pass -DGAME_NAME=MyGame or -DGAME_PATH=/absolute/or/relative/path
set(GAME_NAME "Game" CACHE STRING "Name of the game project subdirectory under source/Games")
set(GAME_PATH "" CACHE PATH "Optional path to the game project directory. If set, overrides GAME_NAME/GAME_DIR")

# ——————————————————————————————————————————————————————————
# Global settings
# ——————————————————————————————————————————————————————————
option(ENABLE_PTHREADS "Enable Emscripten pthread support (WASM threads)" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# We'll put final runtime / archive / library outputs under Build/<GAME_NAME>/<platform>
# (these are set later after we detect platform and GAME_DIR)
# Global compile options (applies to all targets)
add_compile_options(-O2 -DNDEBUG -g) #

if (EMSCRIPTEN AND ENABLE_PTHREADS)
    message(STATUS "Emscripten pthreads: ENABLED")
    add_compile_options(-pthread)   # compile with pthread support
else()
    if (EMSCRIPTEN)
        message(STATUS "Emscripten pthreads: DISABLED")
    endif()
endif()

# ——————————————————————————————————————————————————————————
# Platform detection
# ——————————————————————————————————————————————————————————
if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(EMSCRIPTEN TRUE)
    message(STATUS "Building for Emscripten")
    include_directories(SYSTEM ${EMSCRIPTEN_SYSROOT}/include)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msimd128")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse2 -msimd128 -msse4.2")

elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(LINUX TRUE)
    message(STATUS "Building for Linux")

endif()


# ——————————————————————————————————————————————————————————————————————
# Engine library
# ——————————————————————————————————————————————————————————
file(GLOB_RECURSE ENGINE_SOURCES "${CMAKE_SOURCE_DIR}/source/Engine/*.cpp")

add_library(engine STATIC ${ENGINE_SOURCES})

# Engine must see itself as a root include folder
target_include_directories(engine
    PUBLIC
        "${CMAKE_SOURCE_DIR}/source/Engine"   # <Renderer/...>
        "${CMAKE_SOURCE_DIR}/libraries"       # third-party headers
)
if(LINUX)
    add_library(mimalloc::mimalloc STATIC IMPORTED GLOBAL)
    set_target_properties(mimalloc::mimalloc PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Lib/Linux/libmimalloc.a"
    )

    # Find other Linux dependencies
    find_package(GLEW       REQUIRED)
    find_package(OpenGL     REQUIRED)
    find_package(OpenAL     REQUIRED)
    find_package(ZLIB       REQUIRED)

    # Link OpenAL via imported target
    target_link_libraries(engine PRIVATE OpenAL::OpenAL)

    # Link the rest as before
    target_link_libraries(engine PRIVATE
        GLEW::GLEW
        OpenGL::GL
        ZLIB::ZLIB
        mimalloc::mimalloc 
    )
endif()

set(LIB_DIR "${CMAKE_SOURCE_DIR}/sourceLibraries")

# --------------------------------------------------
# RmlUi (vendored)
# --------------------------------------------------
set(RMLUI_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(RMLUI_BUILD_LUA     OFF CACHE BOOL "" FORCE)
set(RMLUI_ENABLE_DEBUGGER ON CACHE BOOL "" FORCE)

add_subdirectory(
    "${LIB_DIR}/RmlUi-6.1"
)

target_link_libraries(engine PUBLIC
    RmlUi::RmlUi
	RmlUi::Debugger
)
# ——————————————————————————————————————————————————————————

# Force static builds for all vendored libs on all platforms
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# --------------------------------------------------
# SDL2 (vendored)
# --------------------------------------------------
add_subdirectory("${LIB_DIR}/SDL")

target_link_libraries(engine PUBLIC SDL2::SDL2)

# --------------------------------------------------
# SDL2 TTF (vendored)
# --------------------------------------------------
add_subdirectory("${LIB_DIR}/SDL_ttf")

# SDL_ttf only exports SDL2_ttf::SDL2_ttf for shared builds.
# For static builds (including Emscripten), create the alias manually.
if(NOT TARGET SDL2_ttf::SDL2_ttf)
    if(TARGET SDL2_ttf)
        add_library(SDL2_ttf::SDL2_ttf ALIAS SDL2_ttf)
    elseif(TARGET SDL2_ttf-static)
        add_library(SDL2_ttf::SDL2_ttf ALIAS SDL2_ttf-static)
    else()
        # Dump all targets to help debug
        message(FATAL_ERROR 
            "Could not find SDL2_ttf or SDL2_ttf-static after add_subdirectory. "
            "Check what targets SDL_ttf actually exports by searching its CMakeLists.txt "
            "for add_library calls."
        )
    endif()
endif()

target_link_libraries(engine PUBLIC SDL2_ttf::SDL2_ttf)

add_subdirectory(${LIB_DIR}/fmod)
target_link_libraries(engine PUBLIC fmod)



# Jolt shipping configuration

# Jolt shipping configuration
set(ENABLE_ASSERTS              OFF CACHE BOOL "" FORCE)
set(ENABLE_ALL_WARNINGS         OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION ON  CACHE BOOL "" FORCE)

# Disable debug features...
set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE OFF CACHE BOOL "" FORCE)
set(PROFILER_IN_DEBUG_AND_RELEASE       OFF CACHE BOOL "" FORCE)


if(EMSCRIPTEN)
    set(USE_WASM_SIMD ON CACHE BOOL "Enable SIMD on Emscripten WASM build" FORCE)
endif()

add_subdirectory(${LIB_DIR}/JoltPhysics-5.3.0/Build/)
target_link_libraries(engine PUBLIC Jolt::Jolt)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DLL_EXPORT OFF)

set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)   # .obj
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)  # .glb / .gltf

set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "" FORCE)

add_subdirectory(${LIB_DIR}/assimp)
target_link_libraries(engine PUBLIC assimp)


# BGFX

set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if(WIN32)
    set(BGFX_BUILD_TOOLS    ON CACHE BOOL "" FORCE)
else()
    set(BGFX_BUILD_TOOLS    OFF CACHE BOOL "" FORCE)
endif()

if(EMSCRIPTEN)
    set(BGFX_CONFIG_MULTITHREADED OFF CACHE BOOL "" FORCE)
    set(BX_CONFIG_SUPPORTS_THREADING OFF CACHE BOOL "" FORCE)
    set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
endif()

set(BGFX_INSTALL        OFF CACHE BOOL "" FORCE)

add_subdirectory(${LIB_DIR}/bgfx/bgfx.cmake)
target_link_libraries(engine PUBLIC bgfx)
target_link_libraries(engine PUBLIC bx)
if(EMSCRIPTEN)
    if(TARGET bimg_encode)
        # Undefine the macros only for this target so it skips the broken #include <x86intrin.h>
        # Jolt and everything else still get the full SSE4.2 intrinsics.
        target_compile_options(bimg_encode PRIVATE -U__SSE4_1__ -U__SSE4_2__)
        message(STATUS "bimg_encode: disabled SSE4.1/4.2 intrinsics to avoid x86intrin.h errors on Emscripten")
    endif()
endif()

# ——————————————————————————————————————————————————————————
# Resolve GAME_DIR, create source/game folder if missing, save selection
# ——————————————————————————————————————————————————————————
if(GAME_PATH)
    # explicit path override (absolute or relative)
    set(GAME_DIR "${GAME_PATH}")
else()
    # default layout: source/Games/<GAME_NAME>
    set(GAME_DIR "${CMAKE_SOURCE_DIR}/source/Games/${GAME_NAME}")

    # backward compatibility: if that path doesn't exist but old `source/Game` does, fall back
    if(NOT IS_DIRECTORY "${GAME_DIR}" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/source/Game")
        message(WARNING "Game directory '${GAME_DIR}' not found. Falling back to legacy 'source/Game'. Consider passing -DGAME_NAME or -DGAME_PATH to specify the game project.")
        set(GAME_DIR "${CMAKE_SOURCE_DIR}/source/Game")
    endif()
endif()

message(STATUS "Using game source directory: ${GAME_DIR}")

# ——————————————————————————————————————————————————————————
# Common WASM linker flags (for the final module)
# ——————————————————————————————————————————————————————————
set(WASM_COMMON_LINK_OPTIONS
    "SHELL:-s USE_SDL=2"
    "SHELL:-s USE_SDL_IMAGE=2"
    "SHELL:-s SDL2_IMAGE_FORMATS=[\"png\",\"jpg\",\"tga\"]"
    "SHELL:-s USE_WEBGL2=1"
    "SHELL:-s MIN_WEBGL_VERSION=2"
    "SHELL:-s MAX_WEBGL_VERSION=2"


    #"SHELL:-s GL_PREINITIALIZED_CONTEXT=1"
    "SHELL:-s STACK_SIZE=500000"
    #"SHELL:-fsanitize=address,undefined"
    "SHELL:-sFETCH"
    "SHELL:-sASYNCIFY"

    "SHELL:-s NO_DISABLE_EXCEPTION_CATCHING"
    "SHELL:-s WASM=1"
    "SHELL:-s ASSERTIONS=1"
    "SHELL:--preload-file ${GAME_DIR}/GameData@/GameData"
    "SHELL:-s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1"
    "SHELL:-lidbfs.js"
    "SHELL:-msimd128"
    "SHELL:-s EXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue']" # for fmod

    "SHELL:-s AGGRESSIVE_VARIABLE_ELIMINATION=1"
    "SHELL:-s ELIMINATE_DUPLICATE_FUNCTIONS=1"

    "SHELL:-gsource-map"
    "SHELL:--source-map-base http://localhost:8779/Build/ShooterGame/emscripten/bin/"

    #"SHELL:--profiling-funcs"
)
if (EMSCRIPTEN AND ENABLE_PTHREADS)
    # PTHREADS: threads require fixed TOTAL_MEMORY and thread flags. Remove memory-growth usage.
    list(APPEND WASM_COMMON_LINK_OPTIONS
        "SHELL:-pthread"                      # pass -pthread to linker
        "SHELL:-s USE_PTHREADS=1"             # enable wasm threads in emscripten
        "SHELL:-s PTHREAD_POOL_SIZE=16"       # adjust pool size as needed
        #"SHELL:-s PROXY_TO_PTHREAD=1"        # optional: proxy main to a worker
        "SHELL:-s TOTAL_MEMORY=999948288"   
    )
else()
    # When not using pthreads you can allow memory growth (useful for dev)
    list(APPEND WASM_COMMON_LINK_OPTIONS
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s INITIAL_HEAP=83886080"
    )
endif()



# Persist last selected game to build dir so it's easy to query later
file(WRITE "${CMAKE_BINARY_DIR}/last_game.txt" "${GAME_NAME}\n")
message(STATUS "Saved last selected game into: ${CMAKE_BINARY_DIR}/last_game.txt")
# Note: GAME_NAME is also a cached variable and is stored in CMakeCache.txt automatically.

# ——————————————————————————————————————————————————————————
# Create platform-specific build subfolder structure under source-level Build/
# (Build/<GameName>/<platform>), and set global output directories
# ——————————————————————————————————————————————————————————
if(EMSCRIPTEN)
    set(PLATFORM_DIR "emscripten")
elseif(LINUX)
    set(PLATFORM_DIR "linux")
elseif(WIN32)
    set(PLATFORM_DIR "windows")
else()
    # fallback: lower-case system name
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" PLATFORM_DIR)
endif()

set(OUTPUT_BASE_DIR "${CMAKE_SOURCE_DIR}/Build/${GAME_NAME}/${PLATFORM_DIR}")
file(MAKE_DIRECTORY "${OUTPUT_BASE_DIR}")
file(MAKE_DIRECTORY "${OUTPUT_BASE_DIR}/bin" "${OUTPUT_BASE_DIR}/lib" "${OUTPUT_BASE_DIR}/assets")

# set global output dirs (single-config and multi-config)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib")

foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${cfg}" CFGU)
    set("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFGU}" "${OUTPUT_BASE_DIR}/bin")
    set("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFGU}" "${OUTPUT_BASE_DIR}/lib")
    set("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFGU}" "${OUTPUT_BASE_DIR}/lib")
endforeach()

message(STATUS "Build output base: ${OUTPUT_BASE_DIR}")

# Ensure targets created earlier (engine) will emit to the new dirs as well
if(TARGET engine)
    set_target_properties(engine PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/bin"
    )
endif()

# ——————————————————————————————————————————————————————————
# Collect game sources
# ——————————————————————————————————————————————————————————
file(GLOB_RECURSE GAME_SOURCES "${GAME_DIR}/*.cpp")

# Include legacy sources if the old directory exists (transitional support)
set(LEGACY_GAME_DIR "${CMAKE_SOURCE_DIR}/source/Game")
if(IS_DIRECTORY "${LEGACY_GAME_DIR}")
    message(STATUS "Legacy game directory found: Including sources from ${LEGACY_GAME_DIR}")
    file(GLOB_RECURSE LEGACY_GAME_SOURCES "${LEGACY_GAME_DIR}/*.cpp")
    list(APPEND GAME_SOURCES ${LEGACY_GAME_SOURCES})
    list(REMOVE_DUPLICATES GAME_SOURCES)
endif()

# ——————————————————————————————————————————————————————————
# Game executable target (target name follows GAME_NAME)
# ——————————————————————————————————————————————————————————
# sanitize GAME_NAME to create a valid CMake target name (replace non-alnum with '_')
string(REGEX REPLACE "[^A-Za-z0-9_]" "_" GAME_TARGET_NAME "${GAME_NAME}")

# If no sources found, we still create the target so the user can populate the folder.
if (NOT GAME_SOURCES)
    message(FATAL_ERROR "No C++ sources found in ${GAME_DIR}. Cannot build ${GAME_NAME}.")
endif()

add_executable(${GAME_TARGET_NAME} ${GAME_SOURCES})

# include directories for the game
target_include_directories(${GAME_TARGET_NAME}
    PUBLIC
        "${GAME_DIR}"
)


# link engine
target_link_libraries(${GAME_TARGET_NAME} PRIVATE engine)

# Set executable output name: keep 'main' for emscripten builds (compat), otherwise use GAME_NAME
if(EMSCRIPTEN)
    set_target_properties(${GAME_TARGET_NAME} PROPERTIES OUTPUT_NAME "main")
    set_target_properties(${GAME_TARGET_NAME} PROPERTIES SUFFIX ".html")
    message(STATUS "Emscripten build: final module OUTPUT_NAME set to 'main' for web compatibility, with SUFFIX '.html' to generate HTML output")
else()
    set_target_properties(${GAME_TARGET_NAME} PROPERTIES OUTPUT_NAME "${GAME_NAME}")
endif()

# Ensure this game target also emits binaries/libs to our platform-specific output directories
set_target_properties(${GAME_TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/lib"
)

if (EMSCRIPTEN)
    message(STATUS "Applying WASM linker flags to final '${GAME_TARGET_NAME}' target")
    target_link_options(${GAME_TARGET_NAME} PRIVATE ${WASM_COMMON_LINK_OPTIONS})

else()

    target_link_options(${GAME_TARGET_NAME} PRIVATE -pthread -ldl)
    install(TARGETS ${GAME_TARGET_NAME} DESTINATION bin)
endif()