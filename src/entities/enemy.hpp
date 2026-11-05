#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/economy.hpp"
#include "core/stats.hpp"
#include "core/types.hpp"
#include "entities/player.hpp"

class PlayerInventory;

// Stat block for an enemy, wrapping a Stats instance plus scaling bookkeeping.
struct EnemyStats {
    struct StatsData {
        int hitpoints;
        int maxHitpoints;
        float armor;
        float magicArmor;
        int attack;
        int magicAttack;
        int mana;
        int maxMana;
        float critRate;
        float critDamage;
        float dodgeRate;
        float magicAmplifierRate;
        int level;
        float expe;
        Economy economy;
    } data;

    bool scaled = false;

    void scale(float multiplier);
};

struct enemyLore {
    std::string desc;
};

// A concrete enemy instance in the world.
struct Enemy {
    std::string name;
    EnemyStats stats;
    // Debuffs this enemy can inflict on a successful attack.
    std::vector<std::string> inflictedDebuffs;
    // Debuffs currently affecting this enemy.
    std::vector<ActiveDebuff> debuffs;
    // Hidden 0..100 fill per debuff id; combat-only, never saved.
    std::map<std::string, int> debuffMeters;
    std::string id;
};

// Catalog of enemy templates plus random selection.
struct EnemyDatabase {
    struct EnemyTemplate {
        std::string name;
        EnemyStats stats;
        enemyLore lore;
        int minLevel;
        std::vector<std::string> debuffs;
        int difficultyLevel;
        std::vector<LocationType> allowedLocations;
        std::string id{};
    };

    std::vector<EnemyTemplate> templates;

    EnemyDatabase();

    Enemy getRandomEnemy(int difficultyLevel, LocationType locationType);
    Enemy getRandomEnemy(int difficultyLevel);
    const EnemyTemplate* find(const std::string& id) const;
};

// Spawns and scales enemies for encounters.
class EnemyController {
public:
    Enemy encounterEnemy(int difficultyLevel, LocationType locationType = Terrain);
    bool hasEligibleEnemy(int difficultyLevel, LocationType locationType) const;
    Enemy getEnemyByName(const std::string& name, int playerLevel, LocationType locationType = Terrain);

    // Awards currency, experience, and material drops. Drops need a backpack;
    // pass nullptr in tests that only care about currency and experience.
    void enemyGoldExpDrop(Player& player, const Enemy& enemy, PlayerInventory* inventory = nullptr);

private:
    EnemyDatabase enemyDB;

    void enemyScaleLevel(Enemy& enemy, int difficultyLevel, LocationType locationType = Terrain);
};
