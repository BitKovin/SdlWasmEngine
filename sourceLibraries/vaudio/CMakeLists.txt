cmake_minimum_required(VERSION 3.16)

# --------------------------------------------------
# Vercidium Audio (native C SDK)
# --------------------------------------------------
add_library(vaudio_native INTERFACE)

target_include_directories(vaudio_native INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/native/include
)

if(EMSCRIPTEN)
    # No native (wasm-ld linkable) build of vaudio exists — wasm/ ships a
    # separate, self-contained .NET WASM module driven purely from JS
    # (vaudio-wrapper.js / .d.ts), not a C ABI you link into your own module.
    # Nothing to link here; see the note at the bottom of this file.
elseif(ANDROID)
    # ANDROID_ABI is set by the NDK toolchain file to one of:
    # arm64-v8a, armeabi-v7a, x86, x86_64
    target_link_libraries(vaudio_native INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/native/android/${ANDROID_ABI}/libvaudionative.so
    )
elseif(APPLE)
    target_link_libraries(vaudio_native INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/native/mac/libvaudionative.dylib
    )
elseif(WIN32)
    # Link against the import library; the DLL is required at runtime (see
    # vaudio_copy_runtime() below).
    target_link_libraries(vaudio_native INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/native/windows/vaudionative.lib
    )
    set(VAUDIO_RUNTIME_DLL ${CMAKE_CURRENT_LIST_DIR}/native/windows/vaudionative.dll
        CACHE INTERNAL "Path to vaudionative.dll for post-build copying")
elseif(UNIX)
    target_link_libraries(vaudio_native INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/native/linux/libvaudionative.so
    )
endif()

# --------------------------------------------------
# Convenience: copy vaudionative.dll next to a target's output on Windows,
# since it isn't found automatically like a static/import lib.
#   Usage: vaudio_copy_runtime(your_target)
# --------------------------------------------------
function(vaudio_copy_runtime TARGET_NAME)
    if(WIN32)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${VAUDIO_RUNTIME_DLL}
                $<TARGET_FILE_DIR:${TARGET_NAME}>
        )
    endif()
endfunction()

# --------------------------------------------------
# WASM / Emscripten builds
#
# Unlike FMOD, there is no .a/.o for vaudio that wasm-ld can link into your
# Emscripten module. native/include/vaudio.h has no __EMSCRIPTEN__ handling
# at all — it's only implemented for Windows/Linux/macOS/Android.
#
# The wasm/ folder is a completely separate, prebuilt .NET WASM module
# (served as static files: vaudio-wrapper.js + _framework/*.wasm) exposing
# its own JS/TypeScript API (see vaudio-wrapper.d.ts) — functions like
# transmissionForThickness(), MaterialType, Vector, Matrix, etc. It is not
# reachable by calling the C functions declared in vaudio.h.
#
# To use vaudio in a web build of a game whose gameplay code is C/C++, you'd
# need to bridge to that JS API explicitly (e.g. Emscripten's EM_JS / CCall
# / EM_ASM) rather than calling into vaudio.h directly — that bridging layer
# isn't something a CMakeLists.txt can provide for you.
# --------------------------------------------------