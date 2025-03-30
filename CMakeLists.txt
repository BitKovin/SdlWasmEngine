cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)
project(client)

option(JS_ONLY "Compiles to native JS (No WASM)" OFF)

add_definitions(-std=c++11 -O3)

file(GLOB_RECURSE ALL_SOURCES "source/*.cpp")
set(SOURCES "")
foreach(source_file IN LISTS ALL_SOURCES)
    if(NOT source_file MATCHES "source/glm/")
        list(APPEND SOURCES ${source_file})
    endif()
endforeach()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build")
add_executable(a ${SOURCES})

set_target_properties(a PROPERTIES OUTPUT_NAME "main")  # Unified output name

include_directories("${CMAKE_SOURCE_DIR}/librariesEms")
target_link_libraries(a PRIVATE "${CMAKE_SOURCE_DIR}/libsEms/libassimp.a" "${CMAKE_SOURCE_DIR}/libsEms/libIrrXML.a")


    message(STATUS "Setting compilation target to WASM")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(a PRIVATE
    "SHELL:-s USE_SDL=2"
    "SHELL:-s USE_SDL_IMAGE=2"
    "SHELL:-s USE_SDL_MIXER=2"
    "SHELL:-s SDL2_IMAGE_FORMATS=[png]"
    "SHELL:-s USE_WEBGL2=1"
    "SHELL:-s MIN_WEBGL_VERSION=2"
    "SHELL:-s MAX_WEBGL_VERSION=2"
    "SHELL:-s GL_PREINITIALIZED_CONTEXT=1"
    "SHELL:-s STACK_SIZE=500000"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s WASM=1"
    "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/GameData@/GameData"
    "SHELL:-s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1"
)