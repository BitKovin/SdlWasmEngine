# cmake/platforms/windows.cmake
# ——————————————————————————————————————————————————————————
# Windows (desktop) platform configuration.
# Included ONCE by CMakeLists.txt during platform detection.
# Paired toolchain: cmake/toolchains/msvc.cmake
# ——————————————————————————————————————————————————————————

if(_PLATFORM_WINDOWS_INCLUDED)
    return()
endif()
set(_PLATFORM_WINDOWS_INCLUDED TRUE)

set(PLATFORM_DIR "windows")
message(STATUS "Building for Windows (MSVC)")

# OpenAL-Soft is available on Windows
set(PLATFORM_HAS_OPENAL TRUE)


# ——————————————————————————————————————————————————————————
# Hook 1: early compile configuration
# ——————————————————————————————————————————————————————————
function(platform_early_config)
    # MSVC global flags are handled by cmake/compiler_flags.cmake.
    # Add any Windows-specific defines here if needed, e.g.:
    #   add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 2: cache variables before third-party add_subdirectory calls
# ——————————————————————————————————————————————————————————
function(platform_pre_libraries)
    # BGFX — shader compiler tools are only built on Windows
    set(BGFX_BUILD_TOOLS ON CACHE BOOL "" FORCE)
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 3: post-library fixups
# ——————————————————————————————————————————————————————————
function(platform_post_libraries engine_target)
    target_link_libraries(engine PUBLIC SDL2::SDL2main)
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 4: finalise the game executable target
# ——————————————————————————————————————————————————————————
function(platform_configure_game_target game_target)
    set_target_properties(${game_target} PROPERTIES OUTPUT_NAME "${GAME_NAME}")

    # Install rule for desktop Windows
    install(TARGETS ${game_target} DESTINATION bin)
endfunction()
