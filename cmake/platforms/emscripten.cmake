# cmake/platforms/emscripten.cmake
# ——————————————————————————————————————————————————————————
# Emscripten / WebAssembly platform configuration
# Included ONCE by CMakeLists.txt during platform detection.
# Defines four hook functions consumed by the main file:
#   platform_early_config()
#   platform_pre_libraries()
#   platform_post_libraries(engine_target)
#   platform_configure_game_target(game_target)
# ——————————————————————————————————————————————————————————

# Guard against double-inclusion
if(_PLATFORM_EMSCRIPTEN_INCLUDED)
    return()
endif()
set(_PLATFORM_EMSCRIPTEN_INCLUDED TRUE)

set(EMSCRIPTEN TRUE)
set(PLATFORM_DIR "emscripten")
message(STATUS "Building for Emscripten (WASM)")

# OpenAL-Soft is not available under Emscripten; SDL audio is used instead.
set(PLATFORM_HAS_OPENAL FALSE)


# ——————————————————————————————————————————————————————————
# Hook 1: early compile configuration
# ——————————————————————————————————————————————————————————
function(platform_early_config)
    # Expose the Emscripten sysroot headers to all targets.
    include_directories(SYSTEM ${EMSCRIPTEN_SYSROOT}/include)

    # WASM SIMD flags — Emscripten-specific, not covered by compiler_flags.cmake.
    # Propagate to parent scope so they affect every target in the build.
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -msimd128"           PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msimd128"  PARENT_SCOPE)

    # pthread compile flag (link-side flag is added in platform_configure_game_target)
    if(ENABLE_PTHREADS)
        message(STATUS "Emscripten pthreads: ENABLED")
        add_compile_options(-pthread)
    else()
        message(STATUS "Emscripten pthreads: DISABLED")
    endif()

    # NOTE: -O2 / -g / -DNDEBUG are handled by cmake/compiler_flags.cmake
    # (Clang branch), so they are NOT repeated here.
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 2: set cache variables before third-party add_subdirectory calls
# ——————————————————————————————————————————————————————————
function(platform_pre_libraries)
    # Prevent FreeType from finding & linking the Emscripten sysroot libpng/zlib.
    # Those cached .a files lack atomics/bulk-memory and are incompatible with -pthread.
    # FreeType falls back to its built-in stubs — PNG emoji glyphs won't render,
    # which is acceptable for a game engine.
    set(FT_DISABLE_PNG   TRUE CACHE BOOL "Disable sysroot libpng in FreeType"  FORCE)
    set(FT_DISABLE_ZLIB  TRUE CACHE BOOL "Disable sysroot zlib in FreeType"    FORCE)
    set(FT_DISABLE_BZIP2 TRUE CACHE BOOL "" FORCE)

    # JoltPhysics — enable WASM SIMD path
    set(USE_WASM_SIMD ON CACHE BOOL "Enable SIMD on Emscripten WASM build" FORCE)

    # BGFX — Emscripten doesn't need shader tools
    set(BGFX_CONFIG_MULTITHREADED    OFF CACHE BOOL "" FORCE)
    set(BX_CONFIG_SUPPORTS_THREADING OFF CACHE BOOL "" FORCE)
    set(BGFX_BUILD_TOOLS             OFF CACHE BOOL "" FORCE)
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 3: post-library fixups (run after all add_subdirectory calls)
# ——————————————————————————————————————————————————————————
function(platform_post_libraries engine_target)
    # bimg_encode pulls in <x86intrin.h> which does not exist on Emscripten.
    # Undefine the SSE4 macros only for that target; Jolt et al. still get them.
    if(TARGET bimg_encode)
        target_compile_options(bimg_encode PRIVATE -U__SSE4_1__ -U__SSE4_2__)
        message(STATUS "bimg_encode: disabled SSE4.1/4.2 to avoid x86intrin.h on Emscripten")
    endif()
endfunction()


# ——————————————————————————————————————————————————————————
# Hook 4: finalise the game executable target
# ——————————————————————————————————————————————————————————
function(platform_configure_game_target game_target)
    # Web builds must be named 'main' with an HTML wrapper
    set_target_properties(${game_target} PROPERTIES
        OUTPUT_NAME "main"
        SUFFIX      ".html"
    )
    message(STATUS "Emscripten: OUTPUT_NAME='main', SUFFIX='.html'")

    # ---- WASM linker flags ----
    # OUTPUT_BASE_DIR/ENGINE_ROOT/GAME_NAME/SKIP_GAMEDATA_FINALIZE are readable
    # from the calling scope (CMake functions inherit parent-scope variables
    # for reading). When GameData was finalized once, centrally, before any
    # platform build started (SKIP_GAMEDATA_FINALIZE=1 — see
    # BuildScripts/PrepareGameData.cmake), preload directly from that shared
    # location instead of a per-platform copy. This also sidesteps a
    # pre-existing ordering issue: --preload-file needs GameData to already
    # exist AT LINK TIME, but a per-platform finalize only ran in POST_BUILD,
    # i.e. strictly after linking already completed.
    if(SKIP_GAMEDATA_FINALIZE)
        set(_gamedata_preload_dir "${ENGINE_ROOT}/Build/${GAME_NAME}/GameData")
    else()
        set(_gamedata_preload_dir "${OUTPUT_BASE_DIR}/GameData")
    endif()

    set(_link_opts
        "SHELL:-s USE_SDL=2"
        "SHELL:-s USE_WEBGL2=1"
        "SHELL:-s MIN_WEBGL_VERSION=2"
        "SHELL:-s MAX_WEBGL_VERSION=2"

        "SHELL:-s STACK_SIZE=500000"
        "SHELL:-sFETCH"
        "SHELL:-s NO_DISABLE_EXCEPTION_CATCHING"
        "SHELL:-s WASM=1"
        "SHELL:-s ASSERTIONS=1"

        "SHELL:--preload-file ${_gamedata_preload_dir}@/GameData"

        "SHELL:-s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1"
        "SHELL:-lidbfs.js"
        "SHELL:-msimd128"

        # FMOD JS interop
        "SHELL:-s EXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue']"

        "SHELL:-s AGGRESSIVE_VARIABLE_ELIMINATION=1"
        "SHELL:-s ELIMINATE_DUPLICATE_FUNCTIONS=1"
        
        "SHELL:-lwebsocket.js"

        "SHELL:-gsource-map"
        "SHELL:--source-map-base http://localhost:8779/Build/ShooterGame/emscripten/bin/"
    )

    if(ENABLE_PTHREADS)
        list(APPEND _link_opts
            "SHELL:-pthread"
            "SHELL:-s USE_PTHREADS=1"
            "SHELL:-s PTHREAD_POOL_SIZE=16"
            # Fixed memory required when using threads; remove ALLOW_MEMORY_GROWTH
            "SHELL:-s TOTAL_MEMORY=999948288"
        )
    else()
        list(APPEND _link_opts
            "SHELL:-s ALLOW_MEMORY_GROWTH=1"
            "SHELL:-s INITIAL_HEAP=83886080"
        )
    endif()

    message(STATUS "Applying WASM linker flags to '${game_target}'")
    target_link_options(${game_target} PRIVATE ${_link_opts})
endfunction()
