#pragma once

#include <set>
#include <string>
#include <vector>

#include "core/economy.hpp"
#include "core/lore.hpp"
#include "core/progression.hpp"
#include "core/stats.hpp"
#include "core/timesystem.hpp"
#include "core/types.hpp"

// A player-chosen race instance (resolved from a template).
struct PlayerRace {
    std::string name;
    Lore lore;
};

// Template describing a selectable race (stats bonus + lore).
struct PlayerRaceTemplate {
    std::string name;
    Stats statBonus;
    RaceLore lore;
};

// Template describing a selectable class (base stats + lore).
struct PlayerClassTemplate {
    std::string name;
    Stats stats;
    Lore lore;
};

// The player character and all of their mutable state.
class Player {
public:
    std::string name;
    Stats stats;
    Economy economy;
    Progression progression;
    PlayerRace race;
    PlayerClassTemplate playerClass;
    TimeSystem timeSystem;
    std::vector<std::string> debuffs;
    std::vector<std::string> equippedWeaponDebuffs;
    float equippedWeaponDebuffChance;
    LocationType currentLocationType;
    std::string currentLocation;
    std::vector<std::string> learnedSpells;
    std::vector<std::string> equippedStaffSpells;
    bool sleptToday;
    std::set<std::string> defeatedEnemies;
    std::set<std::string> discoveredLocations;
    std::set<std::string> boughtWeapons;
    std::set<std::string> hiredSpecialCharacters;
    std::set<std::string> encounteredEvents;
    bool hasNewDictionaryEntry = false;

    Player(const std::string& n, const PlayerRace& r, const PlayerClassTemplate& pc)
        : name(n), race(r), playerClass(pc), timeSystem(), currentLocationType(Terrain), currentLocation("Abandoned Cell") {
        stats = pc.stats;
        economy = Economy();
        progression = Progression();
    }

    void applyRaceBonus(const Stats& bonus);
    void scaleStats(float multiplier);
};
