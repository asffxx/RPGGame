#include "game/systems/event.hpp"

#include <iostream>
#include <random>

#include "core/ui.hpp"
#include "db/event.hpp"
#include "db/spell.hpp"

void handleEvent(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, NPCGenerator& npcGen) {
    eventDatabase eventDB;
    SpellDatabase spellDB;
    const auto& events = eventDB.getEvents();
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, events.size() - 1);
    const auto& event = events[dist(gen)];

    std::cout << "\n=== EVENT: " << event.name << " ===" << std::endl;
    std::cout << event.description << std::endl;


    if (event.goldReward != 0) {
        if (event.goldReward > 0) {
            hero.economy.addCurrency(0, 0, event.goldReward, 0);
            std::cout << "You gained " << event.goldReward << " silver!" << std::endl;
        } else {
            int goldToSubtract = -event.goldReward;
            if (hero.economy.subtractCurrency(0, 0, goldToSubtract, 0)) {
                std::cout << "You lost " << goldToSubtract << " silver!" << std::endl;
            } else {
                std::cout << "You didn't have enough gold to lose!" << std::endl;
            }
        }
    }

    if (event.expReward != 0) {
        if (event.expReward > 0) {
            hero.stats.expe += event.expReward;
            std::cout << "You gained " << event.expReward << " experience!" << std::endl;
            heroStats.levelUpChecker();
        } else {
            int expToSubtract = -event.expReward;
            hero.stats.expe = std::max<float>(0.0f, hero.stats.expe - static_cast<float>(expToSubtract));
            std::cout << "You lost " << expToSubtract << " experience!" << std::endl;
        }
    }

    if (event.healthEffect != 0) {
        if (event.healthEffect > 0) {
            int heal = std::min(event.healthEffect, hero.stats.maxHitpoints - hero.stats.hitpoints);
            hero.stats.hitpoints += heal;
            std::cout << "You recovered " << heal << " HP!" << std::endl;
        } else {
            int damage = -event.healthEffect;
            hero.stats.hitpoints -= damage;
            if (hero.stats.hitpoints <= 0) {
                hero.stats.hitpoints = 1;
                std::cout << "You took " << damage << " damage but survived!" << std::endl;
            } else {
                std::cout << "You took " << damage << " damage!" << std::endl;
            }
        }
    }

    if (event.manaEffect != 0) {
        if (event.manaEffect > 0) {
            int restore = std::min(event.manaEffect, hero.stats.maxMana - hero.stats.mana);
            hero.stats.mana += restore;
            std::cout << "You recovered " << restore << " Mana!" << std::endl;
        } else {
            int drain = -event.manaEffect;
            hero.stats.mana = std::max<int>(0, hero.stats.mana - drain);
            std::cout << "You lost " << drain << " Mana!" << std::endl;
        }
    }


    if (event.enemyEncounter && !event.enemyName.empty()) {
        std::cout << "\nYou encounter an enemy!" << std::endl;
        Enemy enemy = enemyCtrl.getEnemyByName(event.enemyName, hero.stats.level, hero.currentLocationType);
        std::vector<NPC> emptyParty;
        system("cls");
        CombatScreen combatScreen(hero, emptyParty, enemy, hero.timeSystem, npcGen, spellDB);
        combatScreen.startCombat(combat, playerInventory);
        system("cls");

        if (hero.stats.hitpoints > 0 && enemy.stats.data.hitpoints <= 0) {
            enemyCtrl.enemyGoldExpDrop(hero, enemy);
            heroStats.levelUpChecker();
        }
    }


    if (hero.encounteredEvents.find(event.name) == hero.encounteredEvents.end()) {
        hero.encounteredEvents.insert(event.name);
        hero.hasNewDictionaryEntry = true;
    }

    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
