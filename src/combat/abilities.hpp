#pragma once

#include "entities/player.hpp"
#include "entities/npc.hpp"
#include "entities/enemy.hpp"
#include "data/class_skill.hpp"

struct AbilityState {
    bool used = false;
    bool braced = false;
    bool distracted = false;
    float retaliationMultiplier = 1.0f;
    // Returns zero for a skipped retaliation, one-half for Brace, otherwise one.
    float consumeRetaliation();
};

// False means no turn or resources were consumed. Rolls are injected for testing.
bool useSignature(Player& player, std::vector<NPC>& party, Enemy& enemy,
                  AbilityState& state, float hitRoll, float effectRoll, std::string& log);
