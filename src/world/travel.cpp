#include "travel.hpp"

#include <algorithm>
#include <iostream>
#include <random>

#include "core/ui.hpp"
#include "systems/event.hpp"
#include "app/display.hpp"
#include "encounters.hpp"
#include "quest_system.hpp"

bool recordArrival(GameSession& session, const std::string& location) {
    const auto area = currentArea(location);
    if (area.name != location) return false;
    session.hero.currentLocation = area.name;
    session.hero.currentLocationType = area.type;
    if (!session.travel.discovered.insert(location).second) return false;
    session.hero.stats.expe += 50.0f;
    session.hero.discoveredLocations.insert(location);
    session.hero.hasNewDictionaryEntry = true;
    return true;
}

void TravelSystem::travel(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem) {
    clearScreen();
    std::vector<std::string> lines = {
        "1. Venture into the unknown -- uncover new lands and dangers",
        "2. Follow safe routes -- travel to known, marked locations",
        "3. Follow a lead -- rumor destinations and active quests",
        "4. Back"
    };
    displayBorderedMenu(lines, "Choose an option: ");
    int choice = getNumberInput(1, 4);
    if (choice == 4) return;

    if (choice == 3) {
        auto leads = travelLeads(session);
        if (leads.empty()) { display::message("No leads yet. Listen to tavern rumors or accept a guild quest."); return; }
        std::vector<std::string> names(leads.begin(), leads.end());
        std::vector<std::string> options;
        for (const auto& name : names)
            options.push_back(std::to_string(options.size() + 1) + ". " + name + (state.discovered.count(name) ? " [visited]" : " [not visited]"));
        options.push_back("0. Back");
        displayBorderedMenu(options, "Follow which lead? ");
        int selected = getNumberInput(0, static_cast<int>(names.size()));
        if (selected == 0) return;
        const auto& locations = locationDB.getLocations();
        for (size_t i = 0; i < locations.size(); ++i) if (locations[i].name == names[selected - 1]) {
            enterLocation(hero, enemyCtrl, combat, playerInventory, heroStats, timeSystem, i, state.marked.count(locations[i].name));
            return;
        }
        return;
    }

    if (choice == 1) {
        clearScreen();
        exploreRandomLocation(hero, enemyCtrl, combat, playerInventory, heroStats, timeSystem);
    } else {
        clearScreen();
        const auto& locations = locationDB.getLocations();
        std::vector<size_t> markedLocations;
        for (size_t i = 0; i < locations.size(); ++i) {
            if (state.marked.count(locations[i].name)) {
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
        enterLocation(hero, enemyCtrl, combat, playerInventory, heroStats, timeSystem, idx, true);
    }
}

bool TravelSystem::hasDiscoveredLocations() const {
    return !state.discovered.empty();
}

void TravelSystem::exploreRandomLocation(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem) {
    const auto& locations = locationDB.getLocations();
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, locations.size() - 1);
    size_t idx = dist(gen);
    enterLocation(hero, enemyCtrl, combat, playerInventory, heroStats, timeSystem, idx);
}

void TravelSystem::enterLocation(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem, size_t locationIndex, bool isSafe) {
    const auto& location = locationDB.getLocations()[locationIndex];
    if (recordArrival(session, location.name)) {
        display::message("Discovered " + location.name + ". Gained 50 experience.");
        heroStats.levelUpChecker();
    }
    SpellDatabase spellDB;
    bool inLocation = true;
    while (inLocation) {
        clearScreen();

        bool hasQuest = !questsAtLocation(session).empty();
        display::travelLocationHeader(location.name, location.description, hasQuest);
        int action = getNumberInput(1, hasQuest ? 4 : 3);

        switch (action) {
            case 1: {
                float enemyChance = isSafe ? 0.0f : encounterChance(location.type);
                bool eligible = location.type != PeacefulCity && enemyCtrl.hasEligibleEnemy(location.difficultyLevel, location.type);

                static std::mt19937 gen(std::random_device{}());
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                if (eligible && dist(gen) < enemyChance) {

                    Enemy enemy = enemyCtrl.encounterEnemy(location.difficultyLevel, location.type);
                    CombatScreen combatScreen(hero, party, enemy, timeSystem, npcGen, spellDB);
                    combatScreen.startCombat(combat, playerInventory);

                    if (hero.stats.hitpoints > 0 && enemy.stats.data.hitpoints <= 0) {
                        enemyCtrl.enemyGoldExpDrop(hero, enemy);
                        heroStats.levelUpChecker();
                    }
                } else {
                    handleEvent(hero, enemyCtrl, combat, playerInventory, heroStats, npcGen, party, !isSafe && eligible);
                }
                timeSystem.advanceTime(hero);
                break;
            }
            case 2:
                state.marked.insert(location.name);
                display::travelLocationMarked();
                break;
            case 3:
                inLocation = false;
                break;
            case 4:
                performQuestAction(session, enemyCtrl, combat, npcGen);
                break;
        }
    }
}
