cmake_minimum_required(VERSION 3.16)
project(RPGGame VERSION 3.2.1 LANGUAGES CXX)

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

# NOTE: src/data/enemy_data.cpp is deliberately NOT listed here.
# It is textually #include'd by src/entities/enemy.cpp, so compiling it as a
# separate translation unit produces duplicate-symbol link errors.
add_library(rpg_core
    src/app/display.cpp
    src/app/main_menu.cpp

    src/combat/combat.cpp
    src/combat/abilities.cpp

    src/core/economy.cpp
    src/core/stats.cpp
    src/core/timesystem.cpp
    src/core/ui.cpp

    src/data/class.cpp
    src/data/class_skill.cpp
    src/data/guild.cpp
    src/data/rumor.cpp
    src/data/debuff.cpp
    src/data/equipment.cpp
    src/data/event.cpp
    src/data/food.cpp
    src/data/location.cpp
    src/data/potion.cpp
    src/data/race.cpp
    src/data/spell.cpp

    src/entities/enemy.cpp
    src/entities/npc.cpp
    src/entities/player.cpp

    src/systems/dictionary.cpp
    src/systems/event.cpp
    src/systems/inventory.cpp
    src/systems/party.cpp
    src/systems/save.cpp
    src/systems/guild.cpp
    src/systems/rumor.cpp

    src/ui/menu.cpp
    src/ui/player_controller.cpp

    src/world/establishments.cpp
    src/world/travel.cpp
    src/world/encounters.cpp
    src/world/guild_quest.cpp
    src/world/quest.cpp
    src/world/sidequests.cpp
    src/world/comissions.cpp
    src/world/quest_system.cpp
)

target_include_directories(rpg_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
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
if(BUILD_TESTING)
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
    add_executable(rpg_ui_tests tests/ui_tests.cpp)
    target_link_libraries(rpg_ui_tests PRIVATE rpg_core)
    add_test(NAME terminal_ui COMMAND rpg_ui_tests)
    set_tests_properties(terminal_ui PROPERTIES ENVIRONMENT "TERM=dumb")
    add_test(NAME cli_flows COMMAND ${CMAKE_COMMAND}
        -DRPG_BINARY=$<TARGET_FILE:rpg>
        -DSCRATCH=${CMAKE_CURRENT_BINARY_DIR}/cli-test
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_test.cmake)
    set_tests_properties(cli_flows PROPERTIES TIMEOUT 30)
endif()
