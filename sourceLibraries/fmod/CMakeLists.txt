cmake_minimum_required(VERSION 3.16)

# ====================================================================
# NOTE ON LINUX .so FILES
# ====================================================================
# The FMOD SDK ships lib/x86_64/libfmod.so and libfmod.so.13 (etc.) as
# symlinks to the real, versioned binary (libfmod.so.13.23). In this
# checkout those "symlinks" are broken -- they're small text files
# containing the target name instead of real filesystem symlinks
# (this happens whenever the SDK is unzipped/checked out on a
# Windows-mounted drive under WSL, i.e. DrvFs, which doesn't
# materialize POSIX symlinks the way native Linux does).
#
# Fix applied below (no changes to lib/ or inc/ needed):
#   1. Link directly against the real, versioned .so files.
#   2. Post-build, copy those real files next to the executable under
#      the filenames the dynamic loader actually looks for at runtime
#      (the SONAME baked into each .so, e.g. "libfmod.so.13"), since
#      those exact filenames are also broken placeholders in lib/.
#   3. Set BUILD_RPATH/INSTALL_RPATH to $ORIGIN so the loader checks
#      next to the executable.
# ====================================================================

set(FMOD_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/x86_64)

# Copies a real fmod binary next to `target`'s output, under the
# filename the dynamic loader expects (its SONAME).
function(fmod_stage_runtime_lib real_file soname_file target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${real_file}"
            "$<TARGET_FILE_DIR:${target}>/${soname_file}"
        COMMENT "Staging ${soname_file} next to $<TARGET_FILE_NAME:${target}>"
        VERBATIM
    )
endfunction()

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
    # Link the real, versioned binary directly -- libfmod.so is a
    # broken placeholder on this checkout (see note above).
    target_link_libraries(fmod_core INTERFACE
        ${FMOD_LIB_DIR}/libfmod.so.13.23
    )
    set(FMOD_CORE_REAL_LIB ${FMOD_LIB_DIR}/libfmod.so.13.23 CACHE INTERNAL "")
    set(FMOD_CORE_SONAME   libfmod.so.13                    CACHE INTERNAL "")
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
        ${FMOD_LIB_DIR}/libfmodstudio.so.13.23
    )
    set(FMOD_STUDIO_REAL_LIB ${FMOD_LIB_DIR}/libfmodstudio.so.13.23 CACHE INTERNAL "")
    set(FMOD_STUDIO_SONAME   libfmodstudio.so.13                    CACHE INTERNAL "")
endif()

# CRITICAL: studio depends on core (this is what was missing for Linux shared libs)
target_link_libraries(fmod_studio INTERFACE fmod_core)

# --------------------------------------------------
# Convenience target (core + studio)
# --------------------------------------------------
add_library(fmod INTERFACE)
target_link_libraries(fmod INTERFACE fmod_studio)

# --------------------------------------------------
# fmod_link_and_stage(<target>)
# --------------------------------------------------
# Use this instead of target_link_libraries(<target> PRIVATE fmod) for
# any executable that links FMOD on Linux. It links the interface
# targets as usual, then (Linux only) stages the real .so binaries
# next to the executable under both their SONAME and unversioned name,
# and points the executable's rpath at its own directory so the
# dynamic loader finds them without needing LD_LIBRARY_PATH.
function(fmod_link_and_stage target)
    target_link_libraries(${target} PUBLIC fmod)

    if(UNIX AND NOT EMSCRIPTEN)
        fmod_stage_runtime_lib(${FMOD_CORE_REAL_LIB}   ${FMOD_CORE_SONAME}   ${target})
        fmod_stage_runtime_lib(${FMOD_STUDIO_REAL_LIB} ${FMOD_STUDIO_SONAME} ${target})
        # Some code paths / dlopen() calls may ask for the unversioned name too.
        fmod_stage_runtime_lib(${FMOD_CORE_REAL_LIB}   libfmod.so       ${target})
        fmod_stage_runtime_lib(${FMOD_STUDIO_REAL_LIB} libfmodstudio.so ${target})

        set_target_properties(${target} PROPERTIES
            BUILD_RPATH   "$ORIGIN"
            INSTALL_RPATH "$ORIGIN"
        )
    endif()
endfunction()