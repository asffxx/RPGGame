#pragma once

#include <map>
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
    std::vector<ActiveDebuff> debuffs;
    // Hidden 0..100 fill per debuff id; combat-only, never saved.
    std::map<std::string, int> debuffMeters;
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

    bool isSpecialCharacter(const std::string& name) const;
    const std::vector<std::string>& getSpecialCharacters() const;

private:
    std::set<std::string> lockedNames;
};
