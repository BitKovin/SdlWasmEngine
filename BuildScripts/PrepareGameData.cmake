# PrepareGameData.cmake

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(GAMEDATA_SCRIPT "${CMAKE_SOURCE_DIR}/BuildScripts/finalize_gamedata.py")
set(GAMEDATA_SOURCE "${GAME_DIR}/GameData")

# Output directly as "GameData" so the engine finds it correctly
set(GAMEDATA_DEST "${OUTPUT_BASE_DIR}/GameData")

# Run AFTER the executable is fully linked
add_custom_command(
    TARGET ${GAME_TARGET_NAME}
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E echo "Finalizing Game Data..."

    COMMAND Python3::Interpreter
        "${GAMEDATA_SCRIPT}"
        "${GAMEDATA_SOURCE}"
        "${GAMEDATA_DEST}"

    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"

    VERBATIM
)