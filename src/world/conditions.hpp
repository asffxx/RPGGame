#pragma once

#include <string>

#include "gameplay/datafile.hpp"
#include "gameplay/inventory.hpp"

struct GameSession;

// One condition or effect rule shared by dialogue and the story board.
struct ConditionRule {
    std::string type;
    std::string id;
    std::string state;
    int value = 0;
};

// Kept so existing dialogue call sites and tests read naturally.
using DialogueRule = ConditionRule;

// Parses and validates one rule from dialogue or act JSON. Throws
// std::runtime_error with a player-independent message on invalid data.
ConditionRule parseRule(const datafile::Value& value, bool effect,
                        const PlayerInventory& inventory);

// True when the player has reached a city, including the current location on
// saves made before that arrival was recorded in the travel set.
bool cityVisited(const GameSession& session, const std::string& city);

// Evaluates a condition. `dialogueScope` is the resident id for `choice`
// conditions and may be empty elsewhere.
bool conditionPasses(const GameSession& session, const std::string& dialogueScope,
                     const ConditionRule& rule);

// Player-facing reason a condition failed, matching dialogue's existing text.
std::string conditionFailure(const GameSession& session, const std::string& dialogueScope,
                             const ConditionRule& rule);
