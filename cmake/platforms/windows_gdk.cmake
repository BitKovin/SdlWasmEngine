# cmake/platforms/windows_gdk.cmake
if(_PLATFORM_WINDOWS_GDK_INCLUDED)
    return()
endif()
set(_PLATFORM_WINDOWS_GDK_INCLUDED TRUE)

set(PLATFORM_DIR "windows_gdk")
message(STATUS "Building for Windows PC GDK")

set(PLATFORM_HAS_OPENAL TRUE)

# === Locate GDK ===
if(DEFINED ENV{GRDKLatest})
    set(_GDK_GAMEKIT "$ENV{GRDKLatest}/GameKit")
elseif(DEFINED ENV{GameDKCoreLatest})
    set(_GDK_GAMEKIT "$ENV{GameDKCoreLatest}/GRDK/GameKit")
elseif(DEFINED ENV{GameDKLatest})
    set(_GDK_GAMEKIT "$ENV{GameDKLatest}/GRDK/GameKit")
elseif(DEFINED GDK_ROOT)
    set(_GDK_GAMEKIT "${GDK_ROOT}/GRDK/GameKit")
else()
    message(FATAL_ERROR "Cannot locate GDK. Run from GDK Developer Command Prompt.")
endif()

file(TO_CMAKE_PATH "${_GDK_GAMEKIT}" _GDK_GAMEKIT)
set(_GDK_INCLUDE "${_GDK_GAMEKIT}/Include")
set(_GDK_LIB "${_GDK_GAMEKIT}/Lib/amd64")

message(STATUS "GDK GameKit: ${_GDK_GAMEKIT}")

# === PLATFORM CUSTOM SDL FLAG ===
set(PLATFORM_CUSTOM_SDL ON CACHE BOOL "Use custom prebuilt SDL2 for this platform" FORCE)

# Hook 1: Early config
function(platform_early_config)
    add_compile_definitions(
        _GAMING_DESKTOP
        WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
        __GDK__
        __WINGDK__
        SDL_PLATFORM_GDK
        SDL_PLATFORM_WINGDK
    )

    include_directories(SYSTEM
        "${_GDK_INCLUDE}/um"
        "${_GDK_INCLUDE}/shared"
        "${_GDK_INCLUDE}/winrt"
    )

    include_directories("${CMAKE_SOURCE_DIR}/sourceLibraries/SDL/include")

endfunction()

# Hook 2: Pre libraries
function(platform_pre_libraries)
    set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    
    # Force GDK defines during SDL build
    if(PLATFORM_CUSTOM_SDL)
        set(SDL_GDK ON CACHE BOOL "" FORCE)
        set(SDL_PLATFORM_GDK ON CACHE BOOL "" FORCE)
        set(SDL_PLATFORM_WINGDK ON CACHE BOOL "" FORCE)
    endif()
endfunction()

# Hook 3: Post libraries - Custom SDL handling
function(platform_post_libraries engine_target)
    target_link_directories(${engine_target} PUBLIC "${_GDK_LIB}")


target_link_libraries(${engine_target} PUBLIC
        "${CMAKE_SOURCE_DIR}/Lib/GDK/SDL2main.lib"
        "${CMAKE_SOURCE_DIR}/Lib/GDK/SDL2.lib"
        xgameruntime
    )

endfunction()

# Hook 4: Game target
function(platform_configure_game_target game_target)
    set_target_properties(${game_target} PROPERTIES 
        OUTPUT_NAME "${GAME_NAME}"
        WIN32_EXECUTABLE TRUE
    )
    target_link_directories(${game_target} PRIVATE "${_GDK_LIB}")
endfunction()