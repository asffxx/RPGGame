#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "entities/player.hpp"

// Catalog of selectable player races.
struct PlayerRaceDatabase {
    std::vector<PlayerRaceTemplate> templates;

    PlayerRaceDatabase();
};
