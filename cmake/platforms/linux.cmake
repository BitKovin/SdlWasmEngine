# cmake/platforms/linux.cmake
# ——————————————————————————————————————————————————————————
# Linux platform configuration
# Included ONCE by CMakeLists.txt during platform detection.
# Defines four hook functions consumed by the main file:
#   platform_early_config()
#   platform_pre_libraries()
#   platform_post_libraries(engine_target)
#   platform_configure_game_target(game_target)
# ——————————————————————————————————————————————————————————

if(_PLATFORM_LINUX_INCLUDED)
    return()
endif()
set(_PLATFORM_LINUX_INCLUDED TRUE)

set(LINUX TRUE)
set(PLATFORM_DIR "linux")
message(STATUS "Building for Linux")

# OpenAL-Soft is available and used on Linux
set(PLATFORM_HAS_OPENAL TRUE)


# ——————————————————————————————————————————————————————————
# Hook 1: early compile configuration
# ——————————————————————————————————————————————————————————
function(platform_early_config)
    # No extra compile flags needed on Linux beyond the global ones.
    # Add here if required in the future (e.g. -march=native, sanitisers).
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 2: cache variables before third-party add_subdirectory calls
# ——————————————————————————————————————————————————————————
function(platform_pre_libraries)
    # BGFX — enable shader tooling on Linux desktop
    set(BGFX_BUILD_TOOLS ON CACHE BOOL "" FORCE)

    # No Jolt WASM SIMD on Linux (native x86/ARM SIMD is auto-detected)
    # No BGFX threading overrides needed — defaults are fine
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 3: post-library fixups
# ——————————————————————————————————————————————————————————
function(platform_post_libraries engine_target)
    # Nothing platform-specific needed after subdirectories on Linux.
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 4: finalise the game executable target
# ——————————————————————————————————————————————————————————
function(platform_configure_game_target game_target)
    set_target_properties(${game_target} PROPERTIES OUTPUT_NAME "${GAME_NAME}")

    # pthreads + dynamic-linker required on Linux
    target_link_options(${game_target} PRIVATE -pthread -ldl)

    # Install rule (desktop platforms ship a binary)
    install(TARGETS ${game_target} DESTINATION bin)
endfunction()
