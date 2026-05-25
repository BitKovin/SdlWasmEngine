# cmake/compiler_flags.cmake
# ——————————————————————————————————————————————————————————
# Global optimisation, debug-info, and language-conformance flags.
# Included by CMakeLists.txt right after project().
# Translates the same intent across MSVC and Clang/GCC so that
# all platforms (Linux, Windows, Emscripten, Xbox GDK) compile cleanly.
#
# What each block does:
#   MSVC   — /O2 (optimise), /Zi (PDB debug info), /FS (safe parallel PDB
#             writes with Ninja), /EHsc (standard C++ exceptions),
#             /DNDEBUG, /MP (parallel compilation)
#   Clang/GCC — -O2 -g -DNDEBUG (unchanged from the original)
# ——————————————————————————————————————————————————————————

if(MSVC)
    # ---- Runtime library: static MT / MTd ----
    # Two-layer enforcement so it applies to every target, including vendored libs:
    #
    # Layer 1 — CMake's own mechanism.
    #   Normal variable:  read when any target is created in this scope or a child scope.
    #   Cache variable:   read by add_subdirectory() builds that have their own project().
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        CACHE STRING "MSVC runtime library" FORCE)
    #
    # Layer 2 — raw flag string replacement.
    #   Vendored CMakeLists.txt files sometimes append /MD to CMAKE_CXX_FLAGS_*
    #   directly, ignoring CMAKE_MSVC_RUNTIME_LIBRARY.  Replace /MD(d) with
    #   /MT(d) in every config-specific flag variable so those libs stay consistent.
    foreach(_lang C CXX)
        foreach(_cfg "" _DEBUG _RELEASE _MINSIZEREL _RELWITHDEBINFO)
            set(_var "CMAKE_${_lang}_FLAGS${_cfg}")
            string(REPLACE "/MDd" "/MTd" ${_var} "${${_var}}")
            string(REPLACE "/MD"  "/MT"  ${_var} "${${_var}}")
            set(${_var} "${${_var}}" CACHE STRING "" FORCE)
        endforeach()
    endforeach()

    add_compile_options(
        /O2     # Optimise for speed
        /Zi     # Full debug info into a PDB
        /FS     # Force synchronous PDB writes — required for Ninja parallel builds
        /EHsc   # Standard C++ exception handling
        /MP     # Multi-processor compilation
    )
    add_compile_definitions(NDEBUG)
    add_link_options(/DEBUG)

else()
    # Clang and GCC (Linux, Emscripten)
    # Provide a sensible default if the caller didn't set one.
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "No build type specified — defaulting to Release")
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    endif()

    # Use CMake's config-specific flag variables instead of hardcoding.
    # These are appended to the base CMAKE_CXX_FLAGS by CMake automatically.
    set(CMAKE_C_FLAGS_DEBUG          "-O0 -g"             CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG        "-O0 -g"             CACHE STRING "" FORCE)

    set(CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG"       CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE      "-O3 -DNDEBUG"       CACHE STRING "" FORCE)

    set(CMAKE_C_FLAGS_RELWITHDEBINFO  "-O2 -g -DNDEBUG"   CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG"  CACHE STRING "" FORCE)

    set(CMAKE_C_FLAGS_MINSIZEREL      "-Os -DNDEBUG"      CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_MINSIZEREL    "-Os -DNDEBUG"      CACHE STRING "" FORCE)
endif()