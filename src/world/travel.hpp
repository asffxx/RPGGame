#pragma once

#include <vector>

#include "core/types.hpp"
#include "db/location.hpp"
#include "game/combat/combat.hpp"
#include "game/entities/enemy.hpp"
#include "game/entities/inventory.hpp"
#include "game/ui/player_controller.hpp"
#include "game/entities/player.hpp"
#include "game/entities/npc.hpp"

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
