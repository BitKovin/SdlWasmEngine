# PrepareGameData.cmake

# When ON, this file does NOT run finalize_gamedata.py itself. Instead it
# assumes something external already finalized GameData exactly once, into
# the shared Build/<GAME_NAME>/GameData (see BuildScripts/RunGameDataFinalize.cmake),
# and:
#   - Emscripten: does nothing here — its --preload-file flag
#     (cmake/platforms/emscripten.cmake) points directly at the shared
#     location instead of a per-platform copy.
#   - Every other platform: adds a cheap POST_BUILD directory copy from the
#     shared location into its own OUTPUT_BASE_DIR/GameData, instead of
#     re-running the full (potentially slow) finalize pipeline.
#
# Default OFF, so a standalone single-platform `cmake --build` still
# finalizes its own GameData exactly as before, with no external step
# required. An orchestrator building multiple platforms in one run should
# run BuildScripts/RunGameDataFinalize.cmake once up front and pass
# -DSKIP_GAMEDATA_FINALIZE=1 to every platform's configure step.
option(SKIP_GAMEDATA_FINALIZE
    "Skip running finalize_gamedata.py per-platform; assume Build/<GAME_NAME>/GameData was already finalized once, externally" OFF)

set(GAMEDATA_SCRIPT "${ENGINE_ROOT}/BuildScripts/finalize_gamedata.py")
set(GAMEDATA_SOURCE "${GAME_DIR}/GameData")

# Shared, once-finalized location (see RunGameDataFinalize.cmake). Deliberately
# NOT under OUTPUT_BASE_DIR — this is Build/<GAME_NAME>/GameData, one level up
# from Build/<GAME_NAME>/<PLATFORM_DIR>/.
set(GAMEDATA_SHARED_DIR "${ENGINE_ROOT}/Build/${GAME_NAME}/GameData")

# Per-platform destination — unchanged from before, EXCEPT Android, which has
# no "next to the executable" — there is no executable. Assets only reach the
# APK/AAB by sitting in the Gradle module's src/main/assets/ before its
# merge*Assets task runs, so android/app/jni/CMakeLists.txt sets
# ANDROID_APP_ASSETS_DIR (a cache var, visible here via add_subdirectory)
# before handing off to this project, and app/build.gradle already makes
# merge*Assets depend on the native build (see the comment there) so this
# POST_BUILD copy is guaranteed to finish first.
if(ANDROID AND DEFINED ANDROID_APP_ASSETS_DIR)
    set(GAMEDATA_DEST "${ANDROID_APP_ASSETS_DIR}/GameData")
else()
    set(GAMEDATA_DEST "${OUTPUT_BASE_DIR}/GameData")
endif()

if(SKIP_GAMEDATA_FINALIZE)

    if(EMSCRIPTEN)
        # Nothing to do — the Emscripten link step preloads GAMEDATA_SHARED_DIR
        # directly (cmake/platforms/emscripten.cmake), no copy needed, and no
        # per-platform GameData folder is created for this target.
        message(STATUS "SKIP_GAMEDATA_FINALIZE: Emscripten will preload GameData directly from ${GAMEDATA_SHARED_DIR}")
    else()
        # Cheap copy of the already-finalized shared folder into this
        # platform's own output directory. No Python, no re-finalizing.
        add_custom_command(
            TARGET ${GAME_TARGET_NAME}
            POST_BUILD

            COMMAND ${CMAKE_COMMAND} -E echo "Copying finalized GameData from ${GAMEDATA_SHARED_DIR} ..."
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${GAMEDATA_DEST}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${GAMEDATA_SHARED_DIR}" "${GAMEDATA_DEST}"

            WORKING_DIRECTORY "${ENGINE_ROOT}"
            VERBATIM
        )
    endif()

else()

    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    # Run AFTER the executable is fully linked
    add_custom_command(
        TARGET ${GAME_TARGET_NAME}
        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E echo "Finalizing Game Data..."

        COMMAND Python3::Interpreter
            "${GAMEDATA_SCRIPT}"
            "${GAMEDATA_SOURCE}"
            "${GAMEDATA_DEST}"

        WORKING_DIRECTORY "${ENGINE_ROOT}"

        VERBATIM
    )

endif()
