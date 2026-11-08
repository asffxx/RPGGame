#include "event.hpp"

#include <iostream>
#include <random>

#include "core/ui.hpp"
#include "catalogs/event.hpp"
#include "catalogs/spell.hpp"
#include "app/display.hpp"

void handleEvent(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, NPCGenerator& npcGen, std::vector<NPC>& party, bool allowCombat) {
    eventDatabase eventDB;
    SpellDatabase spellDB;
    const auto& events = eventDB.getEvents();
    static std::mt19937 gen(std::random_device{}());
    std::vector<size_t> eligible;
    for (size_t i = 0; i < events.size(); ++i)
        if (allowCombat || !events[i].enemyEncounter) eligible.push_back(i);
    if (eligible.empty()) return;
    std::uniform_int_distribution<size_t> dist(0, eligible.size() - 1);
    const auto& event = events[eligible[dist(gen)]];

    clearScreen();

    std::vector<std::string> lines;
    lines.push_back("[ Event: " + event.name + " ]");
    lines.push_back(event.description);
    lines.push_back("----------------------------");

    if (event.goldReward != 0) {
        if (event.goldReward > 0) {
            hero.economy.addCurrency(0, 0, event.goldReward, 0);
            lines.push_back("You gained " + std::to_string(event.goldReward) + " silver!");
        } else {
            int goldToSubtract = -event.goldReward;
            if (hero.economy.subtractCurrency(0, 0, goldToSubtract, 0)) {
                lines.push_back("You lost " + std::to_string(goldToSubtract) + " silver!");
            } else {
                lines.push_back("You didn't have enough gold to lose!");
            }
        }
    }

    if (event.expReward != 0) {
        if (event.expReward > 0) {
            hero.stats.expe += event.expReward;
            lines.push_back("You gained " + std::to_string(event.expReward) + " experience!");
        } else {
            int expToSubtract = -event.expReward;
            hero.stats.expe = std::max<float>(0.0f, hero.stats.expe - static_cast<float>(expToSubtract));
            lines.push_back("You lost " + std::to_string(expToSubtract) + " experience!");
        }
    }

    if (event.healthEffect != 0) {
        if (event.healthEffect > 0) {
            int heal = std::min(event.healthEffect, hero.stats.maxHitpoints - hero.stats.hitpoints);
            hero.stats.hitpoints += heal;
            lines.push_back("You recovered " + std::to_string(heal) + " HP!");
        } else {
            int damage = -event.healthEffect;
            hero.stats.hitpoints -= damage;
            if (hero.stats.hitpoints <= 0) {
                hero.stats.hitpoints = 1;
                lines.push_back("You took " + std::to_string(damage) + " damage but survived!");
            } else {
                lines.push_back("You took " + std::to_string(damage) + " damage!");
            }
        }
    }

    if (event.manaEffect != 0) {
        if (event.manaEffect > 0) {
            int restore = std::min(event.manaEffect, hero.stats.maxMana - hero.stats.mana);
            hero.stats.mana += restore;
            lines.push_back("You recovered " + std::to_string(restore) + " Mana!");
        } else {
            int drain = -event.manaEffect;
            hero.stats.mana = std::max<int>(0, hero.stats.mana - drain);
            lines.push_back("You lost " + std::to_string(drain) + " Mana!");
        }
    }

    bool enemyFight = event.enemyEncounter && !event.enemyName.empty();
    if (enemyFight) lines.push_back("You encounter an enemy!");
    display::eventBox(lines);
    display::eventPressEnter();
    if (event.expReward > 0) heroStats.levelUpChecker();

    if (enemyFight) {
        Enemy enemy = enemyCtrl.getEnemyByName(event.enemyName, hero.stats.level, hero.currentLocationType);
        CombatScreen combatScreen(hero, party, enemy, hero.timeSystem, npcGen, spellDB);
        combatScreen.startCombat(combat, playerInventory);

        if (hero.stats.hitpoints > 0 && enemy.stats.data.hitpoints <= 0) {
            enemyCtrl.enemyGoldExpDrop(hero, enemy, &playerInventory);
            heroStats.levelUpChecker();
        }
    }

    if (hero.encounteredEvents.find(event.name) == hero.encounteredEvents.end()) {
        hero.encounteredEvents.insert(event.name);
        hero.hasNewDictionaryEntry = true;
    }

}
