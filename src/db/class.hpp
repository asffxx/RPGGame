#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "game/entities/player.hpp"

// Catalog of selectable player classes.
class PlayerClassCollection {
public:
    std::vector<PlayerClassTemplate> templates;

    PlayerClassCollection();

    const PlayerClassTemplate& getClassTemplate(const std::string& className) const;
};
