cmake_minimum_required(VERSION 3.5.1)
project(client)

option(JS_ONLY "Compiles to native JS (No WASM)" OFF)

add_definitions(-std=c++11 -O3)

file(GLOB SOURCES source/*.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/public")
add_executable(a ${SOURCES})

set_target_properties(a PROPERTIES OUTPUT_NAME "main")  # Unified output name

if(JS_ONLY)
    message(STATUS "Setting compilation target to native JavaScript")
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    target_link_options(a PRIVATE
        "SHELL:-s USE_SDL=2"
        "SHELL:-s USE_SDL_IMAGE=2"
        "SHELL:-s USE_SDL_MIXER=2"
        "SHELL:-s SDL2_IMAGE_FORMATS=[png]"
        "SHELL:-s FULL_ES3=1"
        "SHELL:-s STACK_SIZE=500000"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s WASM=0"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
    )
else()
    message(STATUS "Setting compilation target to WASM")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(a PRIVATE
        "SHELL:-s USE_SDL=2"
        "SHELL:-s USE_SDL_IMAGE=2"
        "SHELL:-s USE_SDL_MIXER=2"
        "SHELL:-s SDL2_IMAGE_FORMATS=[png]"
        "SHELL:-s FULL_ES3=1"
        "SHELL:-s STACK_SIZE=500000"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s WASM=1"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
    )
endif()