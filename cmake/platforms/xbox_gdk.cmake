# cmake/platforms/xbox_gdk.cmake
# ——————————————————————————————————————————————————————————
# Xbox GDK platform configuration.
# Paired toolchain: cmake/toolchains/msvc.cmake
#
# Usage:
#   cmake -G Ninja \
#         -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake \
#         -DGDK_SCARLETT=ON \
#         -B build/xbox_gdk ..
#
# Run from a Visual Studio Developer Command Prompt with the GDK extension
# installed; the GDK installer sets the GameDKLatest environment variable.
# Alternatively pass -DGDK_ROOT=<path> explicitly.
# ——————————————————————————————————————————————————————————

if(_PLATFORM_XBOX_GDK_INCLUDED)
    return()
endif()
set(_PLATFORM_XBOX_GDK_INCLUDED TRUE)

set(PLATFORM_DIR "xbox_gdk")
message(STATUS "Building for Xbox GDK")

# OpenAL-Soft is not available on Xbox; audio goes through XAudio2.
set(PLATFORM_HAS_OPENAL FALSE)

# ——————————————————————————————————————————————————————————
# Console variant — Scarlett (Series X|S) or Xbox One
# ——————————————————————————————————————————————————————————
option(GDK_SCARLETT "Target Xbox Series X|S (Scarlett). OFF targets Xbox One." ON)

# ——————————————————————————————————————————————————————————
# Locate the GDK
# Priority: explicit -DGDK_ROOT > GameDKLatest env > GameDK+GDKVersion env
# ——————————————————————————————————————————————————————————
# ——————————————————————————————————————————————————————————
# Locate the GDK
# Checks env vars in the order the GDK installer actually sets them,
# then falls back to an explicit -DGDK_ROOT if none are found.
#
# Observed env vars (GDK 260400 / VS 2026):
#   GRDKLatest       = <root>\260400\GRDK\          ← points right at GRDK
#   GameDKCoreLatest = <root>\260400\               ← versioned root
#   GameDK           = <root>\                      ← bare install root
# ——————————————————————————————————————————————————————————
if(DEFINED ENV{GRDKLatest})
    # GRDKLatest already points at the GRDK folder directly
    set(_GDK_GAMEKIT "$ENV{GRDKLatest}/GameKit")
elseif(DEFINED ENV{GameDKCoreLatest})
    set(_GDK_GAMEKIT "$ENV{GameDKCoreLatest}/GRDK/GameKit")
elseif(DEFINED ENV{GameDKLatest})
    # Older GDK installers used this name
    set(_GDK_GAMEKIT "$ENV{GameDKLatest}/GRDK/GameKit")
elseif(DEFINED GDK_ROOT)
    set(_GDK_GAMEKIT "${GDK_ROOT}/GRDK/GameKit")
else()
    message(FATAL_ERROR
        "Xbox GDK: cannot locate the GDK.\n"
        "Run cmake from the GDK-extended VS Developer Command Prompt, or pass "
        "-DGDK_ROOT=<path/to/GDK/versioned-folder> to cmake.")
endif()

# Normalise path separators so CMake and MSVC agree
file(TO_CMAKE_PATH "${_GDK_GAMEKIT}" _GDK_GAMEKIT)

set(_GDK_INCLUDE "${_GDK_GAMEKIT}/Include")
set(_GDK_LIB     "${_GDK_GAMEKIT}/Lib/amd64")

message(STATUS "GDK GameKit : ${_GDK_GAMEKIT}")

if(NOT IS_DIRECTORY "${_GDK_GAMEKIT}")
    message(FATAL_ERROR
        "Xbox GDK: GameKit directory not found at '${_GDK_GAMEKIT}'.\n"
        "Check that the GDK is fully installed.")
endif()


# ——————————————————————————————————————————————————————————
# Hook 1: early compile configuration
# ——————————————————————————————————————————————————————————
function(platform_early_config)
    # GDK mandatory definitions
    add_compile_definitions(
        #_GAMING_XBOX                            # marks as Xbox build
        WINAPI_FAMILY=WINAPI_FAMILY_GAMES       # restrict to Games partition APIs
        NOMINMAX                                # avoid min/max macro conflicts
        WIN32_LEAN_AND_MEAN                     # trim rarely-used Windows headers
        _CRT_SECURE_NO_WARNINGS                 # suppress deprecated CRT warnings
        SDL_DISABLE_WINDOWS_IME   
    )

    if(GDK_SCARLETT)
        add_compile_definitions(_GAMING_XBOX_SCARLETT)
        message(STATUS "GDK console variant: Xbox Series X|S (Scarlett)")
    else()
        add_compile_definitions(_GAMING_XBOX_XBOXONE)
        message(STATUS "GDK console variant: Xbox One")
    endif()

    # GDK system headers — must come before any vendored lib includes
    # so the Games partition versions of Windows headers are picked up.
    include_directories(SYSTEM
        "${_GDK_INCLUDE}/um"
        "${_GDK_INCLUDE}/shared"
        "${_GDK_INCLUDE}/winrt"
    )
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 2: cache variables before third-party add_subdirectory calls
# ——————————————————————————————————————————————————————————
function(platform_pre_libraries)
    set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

    # ── SDL2: all subsystems that touch desktop-only Win32 APIs ──────────
    set(SDL_VULKAN       OFF CACHE BOOL "" FORCE)  # vulkan_win32.h → HMONITOR
    set(SDL_OPENGL       OFF CACHE BOOL "" FORCE)
    set(SDL_OPENGLES     OFF CACHE BOOL "" FORCE)
    set(SDL_WASAPI       OFF CACHE BOOL "" FORCE)  # desktop audio
    set(SDL_DIRECTX      OFF CACHE BOOL "" FORCE)
    set(SDL_RENDER_D3D   OFF CACHE BOOL "" FORCE)
    set(SDL_RENDER_D3D11 OFF CACHE BOOL "" FORCE)
    set(SDL_SENSOR       OFF CACHE BOOL "" FORCE)  # sensorsapi.h


    # Prevent SDL2 from using roapi.h (RO_INIT_SINGLETHREADED excluded
    # under WINAPI_FAMILY_GAMES even though the file exists)
    set(HAVE_ROAPI_H 0 CACHE INTERNAL "" FORCE)
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 3: post-library fixups
# ——————————————————————————————————————————————————————————
function(platform_post_libraries engine_target)
    # GDK lib search path — used for all target_link_libraries calls below
    target_link_directories(${engine_target} PUBLIC "${_GDK_LIB}")

    # Core GDK runtime
    target_link_libraries(${engine_target} PUBLIC
        xgameruntime    # GDK game runtime (input, storage, networking, …)
        xaudio2         # Audio (replaces OpenAL)
        pixevt          # PIX GPU/CPU event markers for profiling
    )

    # Standard Win32/D3D12 libs that are available in the Games partition
    target_link_libraries(${engine_target} PUBLIC
        d3d12
        dxgi
        dxguid
    )
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 4: finalise the game executable target
# ——————————————————————————————————————————————————————————
function(platform_configure_game_target game_target)
    set_target_properties(${game_target} PROPERTIES OUTPUT_NAME "${GAME_NAME}")

    # GDK lib path for the executable itself (e.g. for any direct SDK linkage)
    target_link_directories(${game_target} PRIVATE "${_GDK_LIB}")

    # makepkg packaging — uncomment and adjust the layout file path once
    # the GDK project manifest (MicrosoftGame.config + layout) is in place.
    # add_custom_command(TARGET ${game_target} POST_BUILD
    #     COMMAND makepkg pack
    #         /f "${CMAKE_SOURCE_DIR}/xbox/layout.xml"
    #         /d "${OUTPUT_BASE_DIR}/bin"
    #         /pd "${OUTPUT_BASE_DIR}/package"
    #     COMMENT "Packaging ${GAME_NAME} for Xbox")
endfunction()