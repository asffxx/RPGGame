#include "event.hpp"

#include <iostream>
#include <random>

#include "core/ui.hpp"
#include "data/event.hpp"
#include "data/spell.hpp"
#include "app/display.hpp"

void handleEvent(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, NPCGenerator& npcGen) {
    eventDatabase eventDB;
    SpellDatabase spellDB;
    const auto& events = eventDB.getEvents();
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, events.size() - 1);
    const auto& event = events[dist(gen)];

    display::eventHeader(event.name, event.description);


    if (event.goldReward != 0) {
        if (event.goldReward > 0) {
            hero.economy.addCurrency(0, 0, event.goldReward, 0);
            display::eventGoldGained(event.goldReward);
        } else {
            int goldToSubtract = -event.goldReward;
            if (hero.economy.subtractCurrency(0, 0, goldToSubtract, 0)) {
                display::eventGoldLost(goldToSubtract);
            } else {
                display::eventNotEnoughGold();
            }
        }
    }

    if (event.expReward != 0) {
        if (event.expReward > 0) {
            hero.stats.expe += event.expReward;
            display::eventExpGained(event.expReward);
            heroStats.levelUpChecker();
        } else {
            int expToSubtract = -event.expReward;
            hero.stats.expe = std::max<float>(0.0f, hero.stats.expe - static_cast<float>(expToSubtract));
            display::eventExpLost(expToSubtract);
        }
    }

    if (event.healthEffect != 0) {
        if (event.healthEffect > 0) {
            int heal = std::min(event.healthEffect, hero.stats.maxHitpoints - hero.stats.hitpoints);
            hero.stats.hitpoints += heal;
            display::eventHpRecovered(heal);
        } else {
            int damage = -event.healthEffect;
            hero.stats.hitpoints -= damage;
            if (hero.stats.hitpoints <= 0) {
                hero.stats.hitpoints = 1;
                display::eventHpDamage(damage, true);
            } else {
                display::eventHpDamage(damage, false);
            }
        }
    }

    if (event.manaEffect != 0) {
        if (event.manaEffect > 0) {
            int restore = std::min(event.manaEffect, hero.stats.maxMana - hero.stats.mana);
            hero.stats.mana += restore;
            display::eventManaRecovered(restore);
        } else {
            int drain = -event.manaEffect;
            hero.stats.mana = std::max<int>(0, hero.stats.mana - drain);
            display::eventManaLost(drain);
        }
    }


    if (event.enemyEncounter && !event.enemyName.empty()) {
        display::eventEnemyEncounter();
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

    display::eventPressEnter();
}
