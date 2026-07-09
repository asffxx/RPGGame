#include "enemy.hpp"

#include <algorithm>
#include <iostream>
#include <random>

#include "core/types.hpp"

void EnemyStats::scale(float multiplier) {
    data.maxHitpoints = static_cast<int>(std::round(data.maxHitpoints * multiplier));
    data.maxMana = static_cast<int>(std::round(data.maxMana * multiplier));
    data.hitpoints = static_cast<int>(std::round(data.hitpoints * multiplier));
    data.hitpoints = std::min<int>(data.hitpoints, data.maxHitpoints);
    data.mana = static_cast<int>(std::round(data.mana * multiplier));
    data.mana = std::min<int>(data.mana, data.maxMana);
    data.armor *= multiplier;
    data.magicArmor *= multiplier;
    data.attack = static_cast<int>(std::round(data.attack * multiplier));
    data.magicAttack = static_cast<int>(std::round(data.magicAttack * multiplier));
    data.dodgeRate *= multiplier;
    data.critRate *= multiplier;
    data.critDamage *= multiplier;
    data.magicAmplifierRate *= multiplier;
}

Enemy EnemyDatabase::getRandomEnemy(int difficultyLevel, LocationType locationType) {
    static std::mt19937 gen(std::random_device{}());
    std::vector<const EnemyTemplate*> validEnemies;

    for (const auto& tmpl : templates) {
        if (tmpl.difficultyLevel <= difficultyLevel &&
            std::find(tmpl.allowedLocations.begin(), tmpl.allowedLocations.end(), locationType) != tmpl.allowedLocations.end()) {
            validEnemies.push_back(&tmpl);
        }
    }

    if (validEnemies.empty()) {
        throw std::runtime_error("No valid enemies for this difficulty level and location type!");
    }

    std::uniform_int_distribution<size_t> dist(0, validEnemies.size() - 1);
    const EnemyTemplate* chosen = validEnemies[dist(gen)];

    return Enemy{ chosen->name, chosen->stats, chosen->debuffs };
}

Enemy EnemyDatabase::getRandomEnemy(int difficultyLevel) {
    static std::mt19937 gen(std::random_device{}());
    std::vector<const EnemyTemplate*> validEnemies;

    for (const auto& tmpl : templates) {
        if (tmpl.difficultyLevel <= difficultyLevel) {
            validEnemies.push_back(&tmpl);
        }
    }

    if (validEnemies.empty()) {
        throw std::runtime_error("No valid enemies for this difficulty level!");
    }

    std::uniform_int_distribution<size_t> dist(0, validEnemies.size() - 1);
    const EnemyTemplate* chosen = validEnemies[dist(gen)];

    return Enemy{ chosen->name, chosen->stats, chosen->debuffs };
}

Enemy EnemyController::encounterEnemy(int difficultyLevel, LocationType locationType) {
    Enemy enemy = enemyDB.getRandomEnemy(difficultyLevel, locationType);
    enemyScaleLevel(enemy, difficultyLevel, locationType);

    std::cout << "A wild " << enemy.name << " has appeared!" << std::endl;
    return enemy;
}

Enemy EnemyController::getEnemyByName(const std::string& name, int playerLevel, LocationType locationType) {
    for (const auto& tmpl : enemyDB.templates) {
        if (tmpl.name == name) {
            Enemy enemy{ tmpl.name, tmpl.stats, tmpl.debuffs };
            enemyScaleLevel(enemy, playerLevel, locationType);
            return enemy;
        }
    }
    throw std::runtime_error("Enemy not found: " + name);
}

void EnemyController::enemyGoldExpDrop(Player& player, const Enemy& enemy) {
    player.economy.addCurrency(enemy.stats.data.economy.platinum, enemy.stats.data.economy.gold, enemy.stats.data.economy.silver, enemy.stats.data.economy.copper);
    player.stats.expe += enemy.stats.data.expe;

    std::cout << "You have defeated " << enemy.name
         << ", you gained " << enemy.stats.data.economy.platinum << "p " << enemy.stats.data.economy.gold << "g " << enemy.stats.data.economy.silver << "s " << enemy.stats.data.economy.copper << "c"
         << " and " << enemy.stats.data.expe
         << " experience!" << std::endl;

    if (player.defeatedEnemies.find(enemy.name) == player.defeatedEnemies.end()) {
        player.defeatedEnemies.insert(enemy.name);
        player.hasNewDictionaryEntry = true;
    }
}

void EnemyController::enemyScaleLevel(Enemy& enemy, int difficultyLevel, LocationType locationType) {
    if (enemy.stats.scaled || enemy.stats.data.level >= difficultyLevel)
        return;

    float scale = 1.0f + static_cast<float>(difficultyLevel - enemy.stats.data.level) * 0.1f;
    if (scale < 0.5f) scale = 0.5f;

    enemy.stats.scale(scale);

    float locationMultiplier = 1.0f;
    switch (locationType) {
        case PeacefulVillage: locationMultiplier = 0.8f; break;
        case PeacefulTown: locationMultiplier = 0.9f; break;
        case Terrain: locationMultiplier = 1.0f; break;
        case Dungeon: locationMultiplier = 1.2f; break;
        case SpellStore: locationMultiplier = 0.9f; break;
    }
    enemy.stats.scale(locationMultiplier);

    switch (locationType) {
        case PeacefulVillage:
            enemy.stats.data.dodgeRate += 0.05f;
            enemy.stats.data.attack = std::max(0, enemy.stats.data.attack - 2);
            enemy.stats.data.magicAttack = std::max(0, enemy.stats.data.magicAttack - 2);
            break;
        case PeacefulTown:
            enemy.stats.data.dodgeRate += 0.03f;
            enemy.stats.data.attack = std::max(0, enemy.stats.data.attack - 1);
            enemy.stats.data.magicAttack = std::max(0, enemy.stats.data.magicAttack - 1);
            break;
        case Terrain:
            break;
        case Dungeon:
            enemy.stats.data.magicArmor += 0.1f;
            enemy.stats.data.magicAttack += 5;
            enemy.stats.data.dodgeRate = std::max(0.0f, enemy.stats.data.dodgeRate - 0.05f);
            break;
        case SpellStore:
            break;
    }

    enemy.stats.data.level = difficultyLevel;
    enemy.stats.scaled = true;
}

#include "enemy_data.cpp"
