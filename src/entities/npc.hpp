#pragma once

#include <set>
#include <string>
#include <vector>
#include <iterator>

#include "core/stats.hpp"
#include "player.hpp"

// A non-player character (party member, merchant, random encounter).
struct NPC {
    std::string name;
    Stats stats;
    PlayerRace race;
    PlayerClassTemplate playerClass;
    int level;
    std::vector<std::string> inventory;
    std::string equippedWeapon;
    std::string equippedArmor;
    std::vector<std::string> debuffs;
    std::vector<std::string> spells;
    int wagePerWeek;
    std::string story;
    std::vector<std::string> dialogues;
};

// Procedurally generates NPCs with randomized race, class, gear, and backstory.
class NPCGenerator {
public:
    NPCGenerator();

    void lockName(const std::string& name);
    void unlockName(const std::string& name);

    NPC generateNPC(int playerLevel);

private:
    const std::vector<std::string> firstNames;
    const std::vector<std::string> lastNames;
    const std::vector<std::string> specialCharacters;
    std::set<std::string> lockedNames;
};
