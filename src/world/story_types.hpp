#pragma once

#include <string>
#include <vector>

// Persistent story state. Everything here is written by dialogue effects and
// read by dialogue conditions, the story board, and reputation gates.

struct StoryFlag {
    std::string id;
    int value = 0;
};

// One choice the player made, in order. `result` is the short consequence line
// shown on the story board.
struct ChoiceRecord {
    std::string dialogueId;
    std::string choiceId;
    std::string result;
};

struct FactionStanding {
    std::string factionId;
    int standing = 0;
};

inline constexpr int minStanding = -100;
inline constexpr int maxStanding = 100;

struct StoryState {
    std::vector<StoryFlag> flags;
    std::vector<ChoiceRecord> choices;
    std::vector<std::string> knownRecipes;
    std::vector<FactionStanding> factions;

    const StoryFlag* flag(const std::string& id) const {
        for (const auto& entry : flags)
            if (entry.id == id) return &entry;
        return nullptr;
    }

    int standing(const std::string& factionId) const {
        for (const auto& entry : factions)
            if (entry.factionId == factionId) return entry.standing;
        return 0;
    }
};
