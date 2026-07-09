#pragma once

#include "game/combat/combat.hpp"
#include "game/entities/enemy.hpp"
#include "game/entities/inventory.hpp"
#include "game/ui/player_controller.hpp"
#include "game/entities/player.hpp"
#include "game/entities/npc.hpp"

// Resolves a random world event (rewards, penalties, or enemy encounters).
void handleEvent(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, NPCGenerator& npcGen);
