cmake_minimum_required(VERSION 3.15)
project(RPGGame CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all sources except enemy_data.cpp, which is intentionally
# #included at the end of game/enemy.cpp (not a standalone translation unit).
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

list(FILTER GAME_SOURCES EXCLUDE REGEX "enemy_data\\.cpp$")

add_executable(rpg ${GAME_SOURCES})
target_include_directories(rpg PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
