#include "npc.hpp"

#include <algorithm>
#include <map>
#include <random>
#include <stdexcept>
#include "catalogs/class.hpp"
#include "catalogs/equipment.hpp"
#include "catalogs/race.hpp"
#include "catalogs/spell.hpp"
#include "catalogs/content_paths.hpp"
#include "gameplay/datafile.hpp"

namespace {
struct NPCPools {
    std::vector<std::string> firstNames;
    std::vector<std::string> lastNames;
    std::vector<std::string> specialCharacters;
    std::vector<std::string> stories;
    std::map<std::string, std::vector<std::string>> specialDialogues;
    std::vector<std::string> normalDialogues;
    std::vector<std::string> starterInventory;
};

const NPCPools& getNPCPools() {
    static const NPCPools pools = [] {
        NPCPools p;
        datafile::Value root;
        std::string error;
        if (!datafile::loadFile(paths::dataFile("characters/npc_pools.json"), root, error)) {
            throw std::runtime_error("Failed to load npc_pools.json: " + error);
        }
        for (const auto& v : root.at("firstNames").array()) p.firstNames.push_back(v.string());
        for (const auto& v : root.at("lastNames").array()) p.lastNames.push_back(v.string());
        for (const auto& v : root.at("specialCharacters").array()) p.specialCharacters.push_back(v.string());
        for (const auto& v : root.at("stories").array()) p.stories.push_back(v.string());
        for (const auto& entry : root.at("specialDialogues").object()) {
            std::vector<std::string> lines;
            for (const auto& v : entry.second.array()) lines.push_back(v.string());
            p.specialDialogues[entry.first] = std::move(lines);
        }
        for (const auto& v : root.at("normalDialogues").array()) p.normalDialogues.push_back(v.string());
        for (const auto& v : root.at("starterInventory").array()) p.starterInventory.push_back(v.string());
        return p;
    }();
    return pools;
}
}

NPCGenerator::NPCGenerator() {}

void NPCGenerator::lockName(const std::string& name) {
    lockedNames.insert(name);
}

void NPCGenerator::unlockName(const std::string& name) {
    lockedNames.erase(name);
}

bool NPCGenerator::isSpecialCharacter(const std::string& name) const {
    const auto& pools = getNPCPools();
    return std::find(pools.specialCharacters.begin(), pools.specialCharacters.end(), name) != pools.specialCharacters.end();
}

const std::vector<std::string>& NPCGenerator::getSpecialCharacters() const {
    return getNPCPools().specialCharacters;
}

NPC NPCGenerator::generateNPC(int playerLevel) {
    const auto& pools = getNPCPools();
    static std::mt19937 gen(std::random_device{}());

    int minLevel = std::max(1, playerLevel - 2);
    int maxLevel = playerLevel + 2;
    std::uniform_int_distribution<int> levelDist(minLevel, maxLevel);
    int level = levelDist(gen);

    std::uniform_real_distribution<float> specialDist(0.0f, 1.0f);
    bool isSpecial = specialDist(gen) < 0.1f; // 10% chance

    std::string name;
    if (isSpecial) {
        std::uniform_int_distribution<size_t> specialCharDist(0, pools.specialCharacters.size() - 1);
        name = pools.specialCharacters[specialCharDist(gen)];
    } else {
        do {
            std::uniform_int_distribution<size_t> firstDist(0, pools.firstNames.size() - 1);
            std::uniform_int_distribution<size_t> lastDist(0, pools.lastNames.size() - 1);
            name = pools.firstNames[firstDist(gen)] + " " + pools.lastNames[lastDist(gen)];
        } while (lockedNames.count(name));
    }

    lockedNames.insert(name);

    PlayerRaceDatabase raceDB;
    const auto& races = raceDB.templates;
    std::uniform_int_distribution<size_t> raceDist(0, races.size() - 1);
    size_t raceIdx = raceDist(gen);
    PlayerRaceTemplate chosenRace = races[raceIdx];
    PlayerRace npcRace{chosenRace.name, {chosenRace.lore.description}};

    // Random class
    PlayerClassCollection classDB;
    const auto& classes = classDB.templates;
    std::uniform_int_distribution<size_t> classDist(0, classes.size() - 1);
    size_t classIdx = classDist(gen);
    PlayerClassTemplate chosenClass = classes[classIdx];

    NPC npc;
    npc.name = name;
    npc.race = npcRace;
    npc.playerClass = chosenClass;
    npc.level = level;
    npc.stats = chosenClass.stats;

    npc.stats.hitpoints += chosenRace.statBonus.hitpoints;
    npc.stats.maxHitpoints += chosenRace.statBonus.maxHitpoints;
    npc.stats.armor += chosenRace.statBonus.armor;
    npc.stats.magicArmor += chosenRace.statBonus.magicArmor;
    npc.stats.attack += chosenRace.statBonus.attack;
    npc.stats.magicAttack += chosenRace.statBonus.magicAttack;
    npc.stats.mana += chosenRace.statBonus.mana;
    npc.stats.maxMana += chosenRace.statBonus.maxMana;

    float scale = 1.0f + (level - 1) * 0.1f;
    npc.stats.scale(scale);

    EquipmentandWeaponDatabase eqDB;
    const auto& equipment = eqDB.getEquipment();
    std::uniform_int_distribution<size_t> eqDist(0, equipment.size() - 1);
    size_t eqIdx = eqDist(gen);
    const auto& eq = equipment[eqIdx];
    if (eq.type == "Weapon") {
        npc.equippedWeapon = eq.name;
        npc.stats.attack += eq.attackIncrease;
        npc.stats.magicAttack += eq.magicAttackIncrease;
    } else {
        npc.equippedArmor = eq.name;
        npc.stats.armor += eq.defenseIncrease;
        npc.stats.magicArmor += eq.magicDefenseIncrease;
    }

    npc.inventory = pools.starterInventory;

    int baseWage = 10 + level * 2;
    npc.wagePerWeek = isSpecial ? baseWage * 2 : baseWage;

    std::uniform_int_distribution<size_t> storyDist(0, pools.stories.size() - 1);
    npc.story = pools.stories[storyDist(gen)];

    SpellDatabase spellDB;
    const auto& spells = spellDB.getSpells();
    std::vector<std::string> availableSpells;
    for (const auto& spell : spells) {
        if (spell.requiredLevel <= level) {
            availableSpells.push_back(spell.spellName);
        }
    }
    int numSpells = std::min(2, (int)availableSpells.size());
    std::shuffle(availableSpells.begin(), availableSpells.end(), gen);
    for (int i = 0; i < numSpells; ++i) {
        npc.spells.push_back(availableSpells[i]);
    }

    // Assign dialogues
    if (isSpecial) {
        auto it = pools.specialDialogues.find(name);
        if (it != pools.specialDialogues.end()) {
            npc.dialogues = it->second;
        }
    } else {
        std::vector<std::string> normalDialogues = pools.normalDialogues;
        std::shuffle(normalDialogues.begin(), normalDialogues.end(), gen);
        int num = 2 + (gen() % 2);
        for (int i = 0; i < num; ++i) {
            npc.dialogues.push_back(normalDialogues[i]);
        }
    }

    return npc;
}
