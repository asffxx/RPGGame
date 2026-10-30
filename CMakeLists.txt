cmake_minimum_required(VERSION 3.16)
project(RPGGame LANGUAGES CXX)

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
# It is textually #include'd at src/entities/enemy.cpp:144, so compiling it as a
# separate translation unit produces duplicate-symbol link errors.
add_executable(rpg
    src/main.cpp

    src/app/display.cpp
    src/app/main_menu.cpp

    src/combat/combat.cpp

    src/core/economy.cpp
    src/core/stats.cpp
    src/core/timesystem.cpp
    src/core/ui.cpp

    src/data/class.cpp
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

    src/ui/menu.cpp
    src/ui/player_controller.cpp

    src/world/establishments.cpp
    src/world/travel.cpp
)

target_include_directories(rpg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MSVC)
    target_compile_options(rpg PRIVATE /W3)
else()
    target_compile_options(rpg PRIVATE -Wall -Wextra)
endif()
