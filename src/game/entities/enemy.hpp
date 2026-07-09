#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "core/economy.hpp"
#include "core/stats.hpp"
#include "core/types.hpp"
#include "player.hpp"

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
    std::vector<std::string> debuffs;
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
    };

    std::vector<EnemyTemplate> templates;

    EnemyDatabase();

    Enemy getRandomEnemy(int difficultyLevel, LocationType locationType);
    Enemy getRandomEnemy(int difficultyLevel);
};

// Spawns and scales enemies for encounters.
class EnemyController {
public:
    Enemy encounterEnemy(int difficultyLevel, LocationType locationType = Terrain);
    Enemy getEnemyByName(const std::string& name, int playerLevel, LocationType locationType = Terrain);

    void enemyGoldExpDrop(Player& player, const Enemy& enemy);

private:
    EnemyDatabase enemyDB;

    void enemyScaleLevel(Enemy& enemy, int difficultyLevel, LocationType locationType = Terrain);
};
