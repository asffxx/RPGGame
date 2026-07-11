#pragma once

#include <vector>

#include "entities/npc.hpp"
#include "entities/player.hpp"

// Pays weekly wages; drops a member if the player cannot afford them.
void deductWeeklyWages(Player& player, std::vector<NPC>& playerParty);

// Interactive party management (talk, story, kick).
void manageParty(std::vector<NPC>& playerParty, NPCGenerator& npcGen, Player& player);
