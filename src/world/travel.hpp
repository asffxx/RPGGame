#pragma once

#include <vector>

#include "core/types.hpp"
#include "data/location.hpp"
#include "combat/combat.hpp"
#include "entities/enemy.hpp"
#include "systems/inventory.hpp"
#include "ui/player_controller.hpp"
#include "entities/player.hpp"
#include "entities/npc.hpp"

// Handles exploration, random discovery, and entering locations.
class TravelSystem {
public:
    TravelSystem(NPCGenerator& gen, bool debugAllDiscovered = false)
        : npcGen(gen), discovered(locationDB.getLocations().size(), debugAllDiscovered), marked(locationDB.getLocations().size(), false) {}

    void travel(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem);
    bool hasDiscoveredLocations() const;

private:
    NPCGenerator& npcGen;
    locationDatabase locationDB;
    std::vector<bool> discovered;
    std::vector<bool> marked;

    void exploreRandomLocation(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem);
    void enterLocation(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem, size_t locationIndex, bool isSafe = false);
};
