#include "travel.hpp"

#include <algorithm>
#include <iostream>
#include <random>

#include "core/ui.hpp"
#include "systems/event.hpp"
#include "app/display.hpp"

void TravelSystem::travel(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem) {
    system("cls");
    std::vector<std::string> lines = {
        "1. Venture into the unknown -- uncover new lands and dangers",
        "2. Follow safe routes -- travel to known, marked locations"
    };
    displayBorderedMenu(lines, "Choose an option: ");
    int choice = getNumberInput(1, 2);

    if (choice == 1) {
        system("cls");
        exploreRandomLocation(hero, enemyCtrl, combat, playerInventory, heroStats, timeSystem);
    } else {
        system("cls");
        const auto& locations = locationDB.getLocations();
        std::vector<size_t> markedLocations;
        for (size_t i = 0; i < locations.size(); ++i) {
            if (marked[i]) {
                markedLocations.push_back(i);
            }
        }
        if (markedLocations.empty()) {
            display::travelNoMarkedLocations();
            return;
        }
        std::vector<std::string> locLines;
        for (size_t j = 0; j < markedLocations.size(); ++j) {
            size_t i = markedLocations[j];
            locLines.push_back(std::to_string(j + 1) + ". " + locations[i].name);
        }
        display::travelLocationList(locLines);
        int locChoice = getNumberInput(1, static_cast<int>(markedLocations.size()));
        size_t idx = markedLocations[locChoice - 1];
        hero.currentLocation = locations[idx].name;
        hero.currentLocationType = locations[idx].type;
        enterLocation(hero, enemyCtrl, combat, playerInventory, heroStats, timeSystem, idx, true);
    }
}

bool TravelSystem::hasDiscoveredLocations() const {
    return std::any_of(discovered.begin(), discovered.end(), [](bool d){ return d; });
}

void TravelSystem::exploreRandomLocation(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem) {
    const auto& locations = locationDB.getLocations();
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, locations.size() - 1);
    size_t idx = dist(gen);
    bool firstTime = !discovered[idx];
    discovered[idx] = true;
    if (firstTime) {
        hero.stats.expe += 50.0f;
        display::travelDiscovered(locations[idx].name);
        heroStats.levelUpChecker();
        hero.discoveredLocations.insert(locations[idx].name);
        hero.hasNewDictionaryEntry = true;

    }
    hero.currentLocation = locations[idx].name;
    hero.currentLocationType = locations[idx].type;
    enterLocation(hero, enemyCtrl, combat, playerInventory, heroStats, timeSystem, idx);
}

void TravelSystem::enterLocation(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem, size_t locationIndex, bool isSafe) {
    const auto& location = locationDB.getLocations()[locationIndex];
    SpellDatabase spellDB;
    bool inLocation = true;
    while (inLocation) {
        system("cls");

        display::travelLocationHeader(location.name, location.description);
        int action = getNumberInput(1, 3);

        switch (action) {
            case 1: {
                float enemyChance = isSafe ? 0.0f : 0.5f;
                if (!isSafe) {
                    switch (location.type) {
                        case PeacefulVillage: enemyChance = 0.2f; break;
                        case PeacefulTown: enemyChance = 0.3f; break;
                        case Dungeon: enemyChance = 0.8f; break;
                        case Terrain: enemyChance = 0.6f; break;
                        default: enemyChance = 0.5f; break;
                    }
                }

                static std::mt19937 gen(std::random_device{}());
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                if (dist(gen) < enemyChance) {

                    Enemy enemy = enemyCtrl.encounterEnemy(location.difficultyLevel, location.type);
                    std::vector<NPC> emptyParty;
                    NPCGenerator localNpcGen;
                    CombatScreen combatScreen(hero, emptyParty, enemy, timeSystem, localNpcGen, spellDB);
                    combatScreen.startCombat(combat, playerInventory);

                    if (hero.stats.hitpoints > 0 && enemy.stats.data.hitpoints <= 0) {
                        enemyCtrl.enemyGoldExpDrop(hero, enemy);
                        heroStats.levelUpChecker();
                    }
                }

                handleEvent(hero, enemyCtrl, combat, playerInventory, heroStats, npcGen);
                timeSystem.advanceTime(hero);
                break;
            }
            case 2:
                marked[locationIndex] = true;
                display::travelLocationMarked();
                break;
            case 3:
                inLocation = false;
                break;
        }
    }
}
