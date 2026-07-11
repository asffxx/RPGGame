#pragma once

#include "combat/combat.hpp"
#include "entities/enemy.hpp"
#include "systems/inventory.hpp"
#include "ui/player_controller.hpp"
#include "entities/player.hpp"
#include "entities/npc.hpp"

// Resolves a random world event (rewards, penalties, or enemy encounters).
void handleEvent(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, NPCGenerator& npcGen);
