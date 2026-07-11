#pragma once

#include <iostream>

#include "systems/inventory.hpp"
#include "entities/player.hpp"

// Displays player stats and handles level-up progression.
class PlayerController {
public:
    PlayerController(Player& p, PlayerInventory& inv)
        : player(p), inventory(inv), reqAmount(100) {}

    void showStats() const;
    void levelUpChecker();

private:
    Player& player;
    PlayerInventory& inventory;
    float reqAmount;
};
