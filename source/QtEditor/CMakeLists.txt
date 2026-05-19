cmake_minimum_required(VERSION 3.19)
project(QtEditor LANGUAGES CXX)

# ——————————————————————————————————————————————————————————
# Engine — consumed as a static library.
# ENGINE_AS_LIBRARY=ON suppresses the game executable block.
# Adjust the path to wherever your engine root actually lives
# relative to this CMakeLists.txt.
# ——————————————————————————————————————————————————————————


# Set the C++ standard (e.g., 11, 14, 17, 20)
set(CMAKE_CXX_STANDARD 20)

# Ensure the compiler must support this version
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable compiler-specific extensions (optional, for pure ISO C++)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GAME_DIR "../../source/Games/ShooterGame/")

add_definitions(-DJPH_NO_DEBUG)

set(PLATFORM_HAS_OPENAL TRUE)
set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(ENGINE_USE_DYNAMIC_RUNTIME ON  CACHE BOOL "" FORCE)
set(ENGINE_AS_LIBRARY ON CACHE BOOL "" FORCE)
set(ENGINE_DISABLE_COMPILER_FLAGS ON CACHE BOOL "" FORCE)
set(ENGINE_DISABLE_PLATFORM_DETECTION ON CACHE BOOL "" FORCE)
set(WIN32 ON CACHE BOOL "" FORCE)
add_subdirectory("../../" "${CMAKE_BINARY_DIR}/engine")
# 'engine' target (and all its transitive deps) is now available.

file(GLOB_RECURSE GAME_SOURCES "${GAME_DIR}/*.cpp")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

# Recursively collect sources, headers, and UI files
file(GLOB_RECURSE QTEDITOR_SOURCES
    CONFIGURE_DEPENDS
    "*.cpp"
    "*.h"
    "*.ui"
)

# Filter out:
# - anything inside a build directory
# - hidden directories/files starting with '.'
list(FILTER QTEDITOR_SOURCES EXCLUDE REGEX [[(^|/)\.]])
list(FILTER QTEDITOR_SOURCES EXCLUDE REGEX [[(^|/)build(/|$)]])

qt_add_executable(QtEditor
    WIN32
    MACOSX_BUNDLE
    ${QTEDITOR_SOURCES}
)

message(STATUS "Sources: ${QTEDITOR_SOURCES}")

target_include_directories(QtEditor
    PUBLIC "${GAME_DIR}"
)

target_compile_definitions(QtEditor PRIVATE SDL_MAIN_HANDLED)

target_link_libraries(QtEditor
    PRIVATE
        Qt::Core
        Qt::Widgets
        engine          # pulls in all engine headers and vendored libs transitively
)

install(TARGETS QtEditor
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QtEditor
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})