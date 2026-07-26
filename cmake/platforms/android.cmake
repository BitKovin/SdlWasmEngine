# cmake/platforms/android.cmake
# ——————————————————————————————————————————————————————————
# Android platform configuration.
# Included ONCE by CMakeLists.txt during platform detection.
# Paired toolchain: cmake/toolchains/android.cmake (wraps the NDK's own
# android.toolchain.cmake — see that file for ABI/API-level/STL selection).
#
# This file is entered from android/app/jni/CMakeLists.txt, which Gradle's
# externalNativeBuild invokes and which add_subdirectory()'s straight into
# ENGINE_ROOT — see that file for how GAME_NAME reaches this configure.
#
# Defines the same four hook functions consumed by the main file:
#   platform_early_config()
#   platform_pre_libraries()
#   platform_post_libraries(engine_target)
#   platform_configure_game_target(game_target)
# ——————————————————————————————————————————————————————————

if(_PLATFORM_ANDROID_INCLUDED)
    return()
endif()
set(_PLATFORM_ANDROID_INCLUDED TRUE)

set(PLATFORM_DIR "android")
message(STATUS "Building for Android (ABI=${ANDROID_ABI}, platform=${ANDROID_PLATFORM}, STL=${ANDROID_STL})")

# openal-soft has a maintained Android backend (OpenSL ES / AAudio) and
# should cross-compile via the NDK toolchain same as every other vendored
# lib here. Left TRUE for consistency with the other platforms, but —
# unlike Linux/Windows — nobody has build-verified it on Android in this
# repo yet. Flip to FALSE if it gives you trouble; FMOD remains the primary
# audio path either way (see fmod_link_and_stage() in the root CMakeLists).
set(PLATFORM_HAS_OPENAL TRUE)


# ——————————————————————————————————————————————————————————
# Hook 1: early compile configuration
# ——————————————————————————————————————————————————————————
function(platform_early_config)
    # NDK toolchain enables -Werror=format-security by default. Downgrade
    # back to a warning rather than silencing it outright — TODO: fix the
    # ImGui::Text(dynamicString.c_str()) call sites this is catching and
    # remove this override.
    add_compile_options(-Wno-error=format-security)
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 2: cache variables before third-party add_subdirectory calls
# ——————————————————————————————————————————————————————————
function(platform_pre_libraries)
    # Cross-compiling: BGFX's offline tools (shaderc, texturec, geometryc)
    # must run on the HOST that invokes the build, not on Android. Never
    # build them here — build bgfx's tools once for your desktop platform
    # and reuse those binaries for shader compilation.
    set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

    # No Jolt WASM SIMD path here; Jolt auto-detects ARM NEON per-ABI.
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 3: post-library fixups (run after all add_subdirectory calls)
# ——————————————————————————————————————————————————————————
function(platform_post_libraries engine_target)
    # Mirrors the Windows hook: SDL2's CMake build produces an SDL2main
    # target on every platform, including Android — on Android its sources
    # are src/main/android/SDL_android_main.c, the JNI glue that the Java
    # SDLActivity calls into. It has to be linked in or the JNI entry point
    # SDL registers itself with is missing at load time.
    target_link_libraries(${engine_target} PUBLIC SDL2::SDL2main)

    # bgfx's Android GL backend (src/glcontext_egl.cpp) calls EGL functions
    # directly rather than going through SDL, and bgfx.cmake doesn't link
    # libEGL.so for you. GLESv2/GLESv1_CM don't need the same treatment —
    # they ride in as private dependencies of SDL2's own Android build (see
    # EXTRA_LIBS in the SDL configure log) and bgfx resolves the actual GL
    # entry points dynamically via eglGetProcAddress rather than linking
    # libGLESv2.so directly, so EGL is the one real gap.
    target_link_libraries(${engine_target} PUBLIC EGL)
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 4: finalise the game executable target
# ——————————————————————————————————————————————————————————
function(platform_configure_game_target game_target)
    set_target_properties(${game_target} PROPERTIES
        OUTPUT_NAME "main"
    )

    # engine is a STATIC lib linked PRIVATE into this SHARED lib. The linker
    # only pulls .o files out of a static archive to satisfy a symbol some
    # other included .o references -- but SDL_main is only ever found via
    # dlsym() from Java, never referenced in the C++ link graph, so it
    # silently gets left out of libmain.so. Force the whole archive in so
    # SDL_main (and anything else only reached via runtime lookup) survives.
    target_link_options(${game_target} PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:engine> -Wl,--no-whole-archive
    )

    target_link_options(${game_target} PRIVATE -Wl,--no-undefined)

    target_link_options(${game_target} PRIVATE
        -Wl,-z,max-page-size=16384
        -Wl,-z,common-page-size=16384 
    )
endfunction()
