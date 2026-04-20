# PrepareGameData.cmake

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(GAMEDATA_SCRIPT "${CMAKE_SOURCE_DIR}/BuildScripts/finalize_gamedata.py")
set(GAMEDATA_SOURCE "${GAME_DIR}/GameData")

# Output directly as "GameData" so the engine finds it by the correct name
set(GAMEDATA_DEST "${OUTPUT_BASE_DIR}/GameData")

add_custom_target(FinalizeGameDataTarget ALL
    COMMAND Python3::Interpreter "${GAMEDATA_SCRIPT}" "${GAMEDATA_SOURCE}" "${GAMEDATA_DEST}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Finalizing Game Data..."
    VERBATIM
)

if(TARGET ${GAME_TARGET_NAME})
    add_dependencies(${GAME_TARGET_NAME} FinalizeGameDataTarget)
endif()