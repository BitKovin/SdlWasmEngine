cmake_minimum_required(VERSION 3.16)

# Captured here, at parse time, because CMAKE_CURRENT_LIST_DIR inside a
# function() body resolves to whichever file *calls* the function, not
# the file the function was defined in -- functions below that need
# this package's own directory (e.g. fmod_stage_android_jar) use this
# variable instead of CMAKE_CURRENT_LIST_DIR. CACHE INTERNAL (matching
# FMOD_CORE_REAL_LIB and friends further down) because add_subdirectory()
# gives this file its own variable scope; a plain set() here would be
# invisible when the function is later called from the parent scope.
set(FMOD_PACKAGE_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

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

# ====================================================================
# NOTE ON ANDROID
# ====================================================================
# The Android package in this checkout is laid out a little differently
# from the other platforms. Headers follow the same inc/<platform>
# pattern used below for html5/windows/linux (they live at
# inc/android/inc -- one extra "inc" level than the others because
# FMOD's own Android download nests them that way). Libs, however,
# follow the same lib/<arch> pattern used for html5/windows/linux
# further below (lib/x86_64, lib/w32) rather than living under inc/:
# the prebuilt .so files are at lib/android/<abi>/, one folder per
# Android ABI (arm64-v8a, armeabi-v7a, x86, x86_64) -- those folder
# names already match Android's own ABI identifiers exactly, so no
# arch-name translation table is needed the way there might be on
# other platforms. (lib/android/ also has "x86"/"x64" folders holding
# fsbank's host-side Windows tool binaries, not Android runtime libs --
# ignore those; they're irrelevant to on-device linking and covered by
# neither FMOD_ANDROID_SUPPORTED_ABIS below nor anything else in this
# file.)
#
# Two things below are easy to get wrong, and both matter for a build
# that actually runs on-device, not just one that links locally:
#
#   1. CMake considers Android to be UNIX (UNIX is TRUE whenever
#      CMAKE_SYSTEM_NAME is "Android"), so every `elseif(ANDROID)`
#      branch below is placed *before* the existing `elseif(UNIX)`
#      branch, not after it. An `elseif(UNIX)` reached first would
#      silently swallow Android builds and point them at the desktop
#      Linux .so files instead (which don't even exist for this ABI).
#
#   2. Linking against the prebuilt .so (for symbol resolution at
#      compile time) does NOT get it into the APK/AAB. The Android
#      Gradle Plugin only packages the .so files it finds sitting in
#      the per-ABI CMake build output directory alongside the library
#      your own target produces. fmod_link_and_stage() below copies
#      libfmod.so/libfmodstudio.so next to your target's own .so on
#      every build (the same ALL-custom-target mechanism used for
#      Linux above) instead of relying on rpath tricks, which don't
#      apply the same way on Android.
#
# Separately, FMOD's Android package also ships a small Java helper,
# fmod.jar (org.fmod.FMOD), which handles audio-focus and headset
# lifecycle events. CMake only builds native code, so it can't add a
# jar to your app module's Java/Kotlin classpath for you -- see
# fmod_stage_android_jar() and the comment on fmod_link_and_stage()
# further down for what you still need to wire up yourself in
# Gradle/your Activity.
# ====================================================================

if(ANDROID)
    # The NDK's own CMake toolchain file (android.toolchain.cmake, used
    # by Android Studio's externalNativeBuild) sets ANDROID_ABI. CMake's
    # built-in Android cross-compilation support (CMAKE_SYSTEM_NAME
    # Android, invoked without the NDK toolchain file) sets
    # CMAKE_ANDROID_ARCH_ABI instead. Support both.
    if(NOT DEFINED ANDROID_ABI AND DEFINED CMAKE_ANDROID_ARCH_ABI)
        set(ANDROID_ABI ${CMAKE_ANDROID_ARCH_ABI})
    endif()

    set(FMOD_ANDROID_SUPPORTED_ABIS arm64-v8a armeabi-v7a x86 x86_64)
    if(NOT ANDROID_ABI IN_LIST FMOD_ANDROID_SUPPORTED_ABIS)
        message(FATAL_ERROR
            "FMOD: unrecognized/unset Android ABI '${ANDROID_ABI}'. This "
            "checkout only ships prebuilt libs for: ${FMOD_ANDROID_SUPPORTED_ABIS}. "
            "ANDROID_ABI/CMAKE_ANDROID_ARCH_ABI is normally set automatically "
            "by the NDK toolchain -- check how CMake is being invoked, or "
            "restrict your build to the supported ABIs (e.g. abiFilters in "
            "build.gradle).")
    endif()

    set(FMOD_ANDROID_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/inc/android/inc)
    set(FMOD_ANDROID_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/android/${ANDROID_ABI})

    # FMOD ships a "release" build of every lib (libfmod.so) and a
    # "logging" build (libfmodL.so) with parameter validation, verbose
    # logcat error output, and extra assertions -- great for
    # development, but bigger and slower. Never ship the L libs to the
    # Play Store. Defaults to ON for Debug configs, OFF otherwise;
    # override any time with -DFMOD_ANDROID_USE_LOGGING_LIBS=ON/OFF.
    set(_fmod_android_logging_default OFF)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(_fmod_android_logging_default ON)
    endif()
    option(FMOD_ANDROID_USE_LOGGING_LIBS
        "Link FMOD's logging (L) libraries instead of the release libraries on Android"
        ${_fmod_android_logging_default})

    if(FMOD_ANDROID_USE_LOGGING_LIBS)
        set(FMOD_ANDROID_SUFFIX "L")
    else()
        set(FMOD_ANDROID_SUFFIX "")
    endif()
    # Core and studio suffixes must match one another: libfmodstudioL.so's
    # NEEDED entry is the literal string "libfmodL.so", not "libfmod.so",
    # so pairing a release studio lib with a logging core lib (or vice
    # versa) will fail to resolve its dependency at load time.

    message(STATUS "FMOD Android: abi=${ANDROID_ABI} logging_libs=${FMOD_ANDROID_USE_LOGGING_LIBS}")
endif()

set(FMOD_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/x86_64)

# Copies a real fmod binary next to `target`'s output, under the
# filename the dynamic loader expects (its SONAME).
#
# NOTE: this intentionally does NOT use add_custom_command(... POST_BUILD ...).
# POST_BUILD steps only run when the target actually relinks. If the staged
# .so is later deleted (e.g. a clean-output script, packaging step, or you
# just rm it by hand) and you then do an incremental rebuild with no source
# changes, the executable is already "up to date" and POST_BUILD never fires
# -- so the file silently stays missing. Using an ALL custom_target instead
# means this reruns (cheaply -- copy_if_different is a no-op when the file
# already matches) on every single build invocation, guaranteeing the file
# is always there after `cmake --build .`, regardless of relink state.
function(fmod_stage_runtime_lib real_file soname_file target)
    if(NOT EXISTS "${real_file}")
        message(WARNING "FMOD: expected runtime lib not found on disk: ${real_file}")
    endif()

    # Sanitize soname_file (e.g. "libfmod.so.13" -> "libfmod_so_13") for use
    # as part of a target name.
    string(MAKE_C_IDENTIFIER "${target}_stage_${soname_file}" _stage_target)

    add_custom_target(${_stage_target} ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${real_file}"
            "$<TARGET_FILE_DIR:${target}>/${soname_file}"
        COMMENT "Staging ${soname_file} next to $<TARGET_FILE_NAME:${target}>"
        VERBATIM
    )
    add_dependencies(${_stage_target} ${target})
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
elseif(ANDROID)
    # Must come before elseif(UNIX) -- Android is UNIX too. See the
    # "NOTE ON ANDROID" block above.
    target_include_directories(fmod_core INTERFACE
        ${FMOD_ANDROID_INC_DIR}
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
elseif(ANDROID)
    # Must come before elseif(UNIX) -- Android is UNIX too. See the
    # "NOTE ON ANDROID" block above. Unlike desktop Linux, Android's .so
    # SONAME already equals its real filename (no "libfmod.so.13.23"
    # style versioning), so there's no broken-symlink workaround needed
    # here -- just link the real file directly.
    set(FMOD_ANDROID_CORE_REAL_LIB ${FMOD_ANDROID_LIB_DIR}/libfmod${FMOD_ANDROID_SUFFIX}.so CACHE INTERNAL "")
    set(FMOD_ANDROID_CORE_SONAME   libfmod${FMOD_ANDROID_SUFFIX}.so                          CACHE INTERNAL "")

    target_link_libraries(fmod_core INTERFACE ${FMOD_ANDROID_CORE_REAL_LIB})

    if(NOT EXISTS "${FMOD_ANDROID_CORE_REAL_LIB}")
        message(WARNING "FMOD: Android core runtime lib not found on disk: ${FMOD_ANDROID_CORE_REAL_LIB}")
    endif()
    message(STATUS "FMOD Android core: real=${FMOD_ANDROID_CORE_REAL_LIB} soname=${FMOD_ANDROID_CORE_SONAME}")
elseif(UNIX)
    # Link the real, versioned binary directly -- libfmod.so is a
    # broken placeholder on this checkout (see note above).
    target_link_libraries(fmod_core INTERFACE
        ${FMOD_LIB_DIR}/libfmod.so.13.23
    )
    set(FMOD_CORE_REAL_LIB ${FMOD_LIB_DIR}/libfmod.so.13.23 CACHE INTERNAL "")
    set(FMOD_CORE_SONAME   libfmod.so.13                    CACHE INTERNAL "")

    if(NOT EXISTS "${FMOD_CORE_REAL_LIB}")
        message(WARNING "FMOD: core runtime lib not found on disk: ${FMOD_CORE_REAL_LIB}")
    endif()
    message(STATUS "FMOD core: real=${FMOD_CORE_REAL_LIB} soname=${FMOD_CORE_SONAME}")
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
elseif(ANDROID)
    # Must come before elseif(UNIX) -- Android is UNIX too. See the
    # "NOTE ON ANDROID" block above.
    set(FMOD_ANDROID_STUDIO_REAL_LIB ${FMOD_ANDROID_LIB_DIR}/libfmodstudio${FMOD_ANDROID_SUFFIX}.so CACHE INTERNAL "")
    set(FMOD_ANDROID_STUDIO_SONAME   libfmodstudio${FMOD_ANDROID_SUFFIX}.so                          CACHE INTERNAL "")

    target_link_libraries(fmod_studio INTERFACE ${FMOD_ANDROID_STUDIO_REAL_LIB})

    if(NOT EXISTS "${FMOD_ANDROID_STUDIO_REAL_LIB}")
        message(WARNING "FMOD: Android studio runtime lib not found on disk: ${FMOD_ANDROID_STUDIO_REAL_LIB}")
    endif()
    message(STATUS "FMOD Android studio: real=${FMOD_ANDROID_STUDIO_REAL_LIB} soname=${FMOD_ANDROID_STUDIO_SONAME}")
elseif(UNIX)
    target_link_libraries(fmod_studio INTERFACE
        ${FMOD_LIB_DIR}/libfmodstudio.so.13.23
    )
    set(FMOD_STUDIO_REAL_LIB ${FMOD_LIB_DIR}/libfmodstudio.so.13.23 CACHE INTERNAL "")
    set(FMOD_STUDIO_SONAME   libfmodstudio.so.13                    CACHE INTERNAL "")

    if(NOT EXISTS "${FMOD_STUDIO_REAL_LIB}")
        message(WARNING "FMOD: studio runtime lib not found on disk: ${FMOD_STUDIO_REAL_LIB}")
    endif()
    message(STATUS "FMOD studio: real=${FMOD_STUDIO_REAL_LIB} soname=${FMOD_STUDIO_SONAME}")
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

    if(ANDROID)
        # Must be checked before the UNIX branch below -- Android is
        # UNIX too. See the "NOTE ON ANDROID" block near the top of this
        # file for why this stages rather than just relies on rpath.
        if(NOT FMOD_ANDROID_CORE_REAL_LIB OR NOT FMOD_ANDROID_CORE_SONAME)
            message(FATAL_ERROR "FMOD: FMOD_ANDROID_CORE_REAL_LIB/FMOD_ANDROID_CORE_SONAME are unset when staging '${target}' -- "
                                 "the fmod_core ANDROID branch above never ran in this scope. "
                                 "Make sure this CMakeLists.txt is add_subdirectory()'d before fmod_link_and_stage() is called.")
        endif()
        if(NOT FMOD_ANDROID_STUDIO_REAL_LIB OR NOT FMOD_ANDROID_STUDIO_SONAME)
            message(FATAL_ERROR "FMOD: FMOD_ANDROID_STUDIO_REAL_LIB/FMOD_ANDROID_STUDIO_SONAME are unset when staging '${target}'.")
        endif()
        message(STATUS "FMOD: staging Android libs for target '${target}': core=${FMOD_ANDROID_CORE_REAL_LIB} studio=${FMOD_ANDROID_STUDIO_REAL_LIB}")

        # No SONAME-vs-unversioned-name dance and no rpath needed here,
        # unlike desktop Linux above: Android's SONAME already equals
        # each file's real name, and the dynamic linker on Android
        # resolves DT_NEEDED against the full set of native libs
        # packaged with the app rather than an $ORIGIN-relative rpath.
        # The only thing that matters is that these two .so files
        # physically land next to `target`'s own .so in the per-ABI
        # build output directory, since that's what the Android Gradle
        # Plugin scans to decide what gets packaged into the APK/AAB.
        fmod_stage_runtime_lib(${FMOD_ANDROID_CORE_REAL_LIB}   ${FMOD_ANDROID_CORE_SONAME}   ${target})
        fmod_stage_runtime_lib(${FMOD_ANDROID_STUDIO_REAL_LIB} ${FMOD_ANDROID_STUDIO_SONAME} ${target})
    elseif(UNIX AND NOT EMSCRIPTEN)
        if(NOT FMOD_CORE_REAL_LIB OR NOT FMOD_CORE_SONAME)
            message(FATAL_ERROR "FMOD: FMOD_CORE_REAL_LIB/FMOD_CORE_SONAME are unset when staging '${target}' -- "
                                 "the fmod_core UNIX branch above never ran in this scope. "
                                 "Make sure this CMakeLists.txt is add_subdirectory()'d before fmod_link_and_stage() is called.")
        endif()
        if(NOT FMOD_STUDIO_REAL_LIB OR NOT FMOD_STUDIO_SONAME)
            message(FATAL_ERROR "FMOD: FMOD_STUDIO_REAL_LIB/FMOD_STUDIO_SONAME are unset when staging '${target}'.")
        endif()
        message(STATUS "FMOD: staging for target '${target}': core=${FMOD_CORE_REAL_LIB} studio=${FMOD_STUDIO_REAL_LIB}")

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

# --------------------------------------------------
# fmod_stage_android_jar(<dest_dir>)
# --------------------------------------------------
# FMOD's Android integration also ships a small Java helper, fmod.jar
# (package org.fmod), that handles audio-focus changes, headset
# plug/unplug broadcasts, and a MediaCodec-based decoder plugin. CMake
# only builds native code, so it can't add this to your app module's
# Java/Kotlin classpath for you -- call this with wherever your Gradle
# module expects prebuilt jars (commonly `<app module>/libs`) to have
# it copied there automatically on every configure. This is optional:
# skip it and copy fmod.jar by hand if you'd rather not point CMake at
# your app module's source tree.
#
# Whichever way the jar gets there, you still need to, in build.gradle:
#     dependencies { implementation files('libs/fmod.jar') }
# and, in your Activity (wherever you initialize FMOD):
#     org.fmod.FMOD.init(this)   // onCreate()  -- audio focus, headset,
#                                                   lifecycle handling
#     org.fmod.FMOD.close()      // onDestroy()
# If you're using a plain NativeActivity with no Java code of your own,
# skip the jar entirely and instead call FMOD_Android_JNI_Init(vm,
# activity) / FMOD_Android_JNI_Close() from native code (fmod_android.h)
# -- see the sample call in that header for attaching the JNI env.
function(fmod_stage_android_jar dest_dir)
    if(ANDROID)
        set(_fmod_jar ${FMOD_PACKAGE_DIR}/lib/android/fmod.jar)
        if(NOT EXISTS "${_fmod_jar}")
            message(WARNING "FMOD: fmod.jar not found on disk: ${_fmod_jar}")
            return()
        endif()
        file(MAKE_DIRECTORY "${dest_dir}")
        file(COPY "${_fmod_jar}" DESTINATION "${dest_dir}")
        message(STATUS "FMOD: staged fmod.jar -> ${dest_dir}/fmod.jar")
    endif()
endfunction()