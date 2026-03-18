cmake_minimum_required(VERSION 3.16)

# --------------------------------------------------
# Core FMOD
# --------------------------------------------------
add_library(fmod_core INTERFACE)

if (EMSCRIPTEN)
    set(FMOD_SUFFIX "")
    if (ENABLE_PTHREADS)
        set(FMOD_SUFFIX "P")
    endif()
endif()

if(EMSCRIPTEN)
    target_include_directories(fmod_core INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/inc/html5
    )
elseif(WIN32)
    target_include_directories(fmod_core INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/inc/windows
    )
elseif(UNIX)
    target_include_directories(fmod_core INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/inc/linux
    )
endif()

target_include_directories(fmod_core INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

if(EMSCRIPTEN)
    target_link_libraries(fmod_core INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/w32/fmod${FMOD_SUFFIX}_wasm.a
    )
elseif(WIN32)
    target_link_libraries(fmod_core INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/x86_64/fmod_vc.lib
    )
elseif(UNIX)
    target_link_libraries(fmod_core INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/x86_64/libfmod.so
    )
endif()

# --------------------------------------------------
# FMOD Studio
# --------------------------------------------------
add_library(fmod_studio INTERFACE)

target_include_directories(fmod_studio INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

if(EMSCRIPTEN)
    target_link_libraries(fmod_studio INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/w32/fmodstudio${FMOD_SUFFIX}_wasm.a
    )
elseif(WIN32)
    target_link_libraries(fmod_studio INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/x86_64/fmodstudio_vc.lib
    )
elseif(UNIX)
    target_link_libraries(fmod_studio INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/x86_64/libfmodstudio.so
    )
endif()

# CRITICAL: studio depends on core (this is what was missing for Linux shared libs)
target_link_libraries(fmod_studio INTERFACE fmod_core)

# --------------------------------------------------
# Convenience target (core + studio)
# --------------------------------------------------
add_library(fmod INTERFACE)
target_link_libraries(fmod INTERFACE fmod_studio)