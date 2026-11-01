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
#include "travel_state.hpp"
#include "app/session.hpp"

// Returns true only on the first physical visit (and grants discovery credit).
bool recordArrival(GameSession& session, const std::string& location);

// Handles exploration, random discovery, and entering locations.
class TravelSystem {
public:
    TravelSystem(NPCGenerator& gen, GameSession& game, bool debugAllDiscovered = false)
        : npcGen(gen), session(game), state(game.travel), party(game.party) {
        if (debugAllDiscovered)
            for (const auto& location : locationDB.getLocations()) state.discovered.insert(location.name);
    }

    void travel(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem);
    bool hasDiscoveredLocations() const;

private:
    NPCGenerator& npcGen;
    GameSession& session;
    locationDatabase locationDB;
    TravelState& state;
    std::vector<NPC>& party;

    void exploreRandomLocation(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem);
    void enterLocation(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem, size_t locationIndex, bool isSafe = false);
};
