cmake_minimum_required(VERSION 3.16)
project(RPGGame VERSION 4.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json so clangd and other language servers can
# resolve the project's include paths and compiler flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to an optimised build when no build type is given.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_library(rpg_core
    src/app/display.cpp
    src/app/main_menu.cpp

    src/combat/combat.cpp
    src/combat/abilities.cpp

    src/core/economy.cpp
    src/core/stats.cpp
    src/core/timesystem.cpp
    src/core/ui.cpp

    src/catalogs/content_ids.cpp
    src/catalogs/content_paths.cpp
    src/catalogs/class.cpp
    src/catalogs/class_skill.cpp
    src/catalogs/guild.cpp
    src/catalogs/rumor.cpp
    src/catalogs/debuff.cpp
    src/catalogs/equipment.cpp
    src/catalogs/event.cpp
    src/catalogs/food.cpp
    src/catalogs/location.cpp
    src/catalogs/material.cpp
    src/catalogs/potion.cpp
    src/catalogs/race.cpp
    src/catalogs/spell.cpp

    src/entities/enemy.cpp
    src/entities/npc.cpp
    src/entities/player.cpp

    src/gameplay/datafile.cpp
    src/gameplay/dictionary.cpp
    src/gameplay/event.cpp
    src/gameplay/inventory.cpp
    src/gameplay/party.cpp
    src/gameplay/save.cpp
    src/gameplay/guild.cpp
    src/gameplay/rumor.cpp

    src/ui/menu.cpp
    src/ui/player_controller.cpp

    src/world/establishments.cpp
    src/world/crafting.cpp
    src/world/conditions.cpp
    src/world/dialogue.cpp
    src/world/reputation.cpp
    src/world/status_effects.cpp
    src/world/story_board.cpp
    src/world/upgrades.cpp
    src/world/travel.cpp
    src/world/encounters.cpp
    src/world/guild_quest.cpp
    src/world/quest.cpp
    src/world/sidequests.cpp
    src/world/commissions.cpp
    src/world/quest_system.cpp
)

target_include_directories(rpg_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(rpg_core PRIVATE RPG_CONTENT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/content")
add_executable(rpg src/main.cpp)
target_link_libraries(rpg PRIVATE rpg_core)

if(MSVC)
    target_compile_options(rpg_core PRIVATE /W3)
    target_compile_options(rpg PRIVATE /W3)
else()
    target_compile_options(rpg_core PRIVATE -Wall -Wextra)
    target_compile_options(rpg PRIVATE -Wall -Wextra)
endif()

include(CTest)
if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/game_tests.cpp")
    add_executable(rpg_dialogue_tests tests/dialogue_tests.cpp)
    target_link_libraries(rpg_dialogue_tests PRIVATE rpg_core)
    foreach(group catalogs branches effects conditions rewards menus)
        add_test(NAME dialogue_${group} COMMAND rpg_dialogue_tests ${group} ${CMAKE_CURRENT_BINARY_DIR}/dialogue-tests/${group})
        set_tests_properties(dialogue_${group} PROPERTIES ENVIRONMENT "TERM=dumb" TIMEOUT 20)
    endforeach()
    add_executable(rpg_story_tests tests/story_tests.cpp)
    target_link_libraries(rpg_story_tests PRIVATE rpg_core)
    foreach(group catalogs beats threads choices menus reload)
        add_test(NAME story_${group} COMMAND rpg_story_tests ${group} ${CMAKE_CURRENT_BINARY_DIR}/story-tests/${group})
        set_tests_properties(story_${group} PROPERTIES ENVIRONMENT "TERM=dumb" TIMEOUT 20)
    endforeach()
    add_executable(rpg_catalog_snapshot tests/catalog_snapshot.cpp)
    target_link_libraries(rpg_catalog_snapshot PRIVATE rpg_core)
    add_executable(rpg_crafting_tests tests/crafting_tests.cpp)
    target_link_libraries(rpg_crafting_tests PRIVATE rpg_core)
    foreach(group catalogs transactions backpack rewards menus)
        add_test(NAME crafting_${group} COMMAND rpg_crafting_tests ${group} ${CMAKE_CURRENT_BINARY_DIR}/crafting-tests/${group})
        set_tests_properties(crafting_${group} PROPERTIES ENVIRONMENT "TERM=dumb" TIMEOUT 15)
    endforeach()
    add_executable(rpg_tests tests/game_tests.cpp)
    target_link_libraries(rpg_tests PRIVATE rpg_core)
    add_test(NAME save_roundtrip COMMAND rpg_tests save ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME class_abilities COMMAND rpg_tests abilities)
    add_test(NAME location_encounters COMMAND rpg_tests encounters)
    add_test(NAME combat_menus COMMAND rpg_tests combat)
    set_tests_properties(combat_menus PROPERTIES ENVIRONMENT "TERM=dumb" TIMEOUT 10)
    add_executable(rpg_guild_tests tests/guild_tests.cpp)
    target_link_libraries(rpg_guild_tests PRIVATE rpg_core)
    add_test(NAME guild_progression COMMAND rpg_guild_tests progression)
    add_test(NAME tavern_rumors COMMAND rpg_guild_tests rumors)
    add_test(NAME guild_save_migration COMMAND rpg_guild_tests save ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME guild_menu_flows COMMAND rpg_guild_tests menus)
    set_tests_properties(guild_menu_flows PROPERTIES ENVIRONMENT "TERM=dumb" TIMEOUT 20)
    add_executable(rpg_quest_tests tests/quest_tests.cpp)
    target_link_libraries(rpg_quest_tests PRIVATE rpg_core)
    add_test(NAME quest_catalog COMMAND rpg_quest_tests catalog)
    add_test(NAME quest_unlock_rules COMMAND rpg_quest_tests rules)
    add_test(NAME quest_lifecycle COMMAND rpg_quest_tests lifecycle)
    add_test(NAME quest_save_migration COMMAND rpg_quest_tests save ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME quest_menu_flows COMMAND rpg_quest_tests menus)
    set_tests_properties(quest_menu_flows PROPERTIES ENVIRONMENT "TERM=dumb" TIMEOUT 20)
    add_executable(rpg_data_tests tests/data_tests.cpp)
    target_link_libraries(rpg_data_tests PRIVATE rpg_core)
    add_test(NAME catalog_ids COMMAND rpg_data_tests ids)
    add_test(NAME json_reader COMMAND rpg_data_tests json ${CMAKE_CURRENT_BINARY_DIR}/data-tests)
    add_test(NAME story_save COMMAND rpg_data_tests save ${CMAKE_CURRENT_BINARY_DIR}/data-tests)
    add_executable(rpg_ui_tests tests/ui_tests.cpp)
    target_link_libraries(rpg_ui_tests PRIVATE rpg_core)
    add_test(NAME terminal_ui COMMAND rpg_ui_tests)
    set_tests_properties(terminal_ui PROPERTIES ENVIRONMENT "TERM=dumb")

    add_executable(rpg_reputation_tests tests/reputation_tests.cpp)
    target_link_libraries(rpg_reputation_tests PRIVATE rpg_core)
    add_test(NAME reputation_catalog COMMAND rpg_reputation_tests catalog ${CMAKE_CURRENT_BINARY_DIR}/reputation-tests/catalog)
    add_test(NAME reputation_tiers COMMAND rpg_reputation_tests tiers ${CMAKE_CURRENT_BINARY_DIR}/reputation-tests/tiers)
    add_test(NAME reputation_dialogue COMMAND rpg_reputation_tests dialogue ${CMAKE_CURRENT_BINARY_DIR}/reputation-tests/dialogue)
    add_test(NAME reputation_quests COMMAND rpg_reputation_tests quests ${CMAKE_CURRENT_BINARY_DIR}/reputation-tests/quests)
    add_test(NAME reputation_shops COMMAND rpg_reputation_tests shops ${CMAKE_CURRENT_BINARY_DIR}/reputation-tests/shops)
    add_test(NAME reputation_display COMMAND rpg_reputation_tests display ${CMAKE_CURRENT_BINARY_DIR}/reputation-tests/display)
    add_test(NAME reputation_save COMMAND rpg_reputation_tests save ${CMAKE_CURRENT_BINARY_DIR}/reputation-tests/save)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/phase7_tests.cpp")
        add_executable(rpg_phase7_tests tests/phase7_tests.cpp)
        target_link_libraries(rpg_phase7_tests PRIVATE rpg_core)
        foreach(group statuses equipment economy balance catalog)
            add_test(NAME phase7_${group} COMMAND rpg_phase7_tests ${group} ${CMAKE_CURRENT_BINARY_DIR}/phase7-tests/${group})
            set_tests_properties(phase7_${group} PROPERTIES ENVIRONMENT "TERM=dumb" TIMEOUT 20)
        endforeach()
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/phase8_tests.cpp")
        add_executable(rpg_phase8_tests tests/phase8_tests.cpp)
        target_link_libraries(rpg_phase8_tests PRIVATE rpg_core)
        foreach(group meters resistance combat)
            add_test(NAME phase8_${group} COMMAND rpg_phase8_tests ${group} ${CMAKE_CURRENT_BINARY_DIR}/phase8-tests/${group})
            set_tests_properties(phase8_${group} PROPERTIES ENVIRONMENT "TERM=dumb" TIMEOUT 20)
        endforeach()
    endif()

    add_test(NAME cli_flows COMMAND ${CMAKE_COMMAND}
        -DRPG_BINARY=$<TARGET_FILE:rpg>
        -DSCRATCH=${CMAKE_CURRENT_BINARY_DIR}/cli-test
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_test.cmake)
    set_tests_properties(cli_flows PROPERTIES TIMEOUT 30)
endif()
