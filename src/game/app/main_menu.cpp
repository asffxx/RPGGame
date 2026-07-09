#include "main_menu.hpp"

#include <iostream>
#include <map>
#include <random>
#include <vector>

#include "core/types.hpp"
#include "core/ui.hpp"
#include "db/spell.hpp"
#include "../entities/npc.hpp"
#include "../combat/combat.hpp"
#include "../systems/dictionary.hpp"
#include "../entities/enemy.hpp"
#include "game/systems/establishments.hpp"
#include "../systems/event.hpp"
#include "game/entities/inventory.hpp"
#include "../ui/menu.hpp"
#include "../systems/party.hpp"
#include "../ui/player_controller.hpp"
#include "world/travel.hpp"

void mainMenu(Player& hero, bool debugMode) {
    EnemyController enemyCtrl;
    CombatSystem combat;
    PlayerInventory playerInventory;
    PlayerController heroStats(hero, playerInventory);
    Store store(playerInventory);
    std::vector<NPC> playerParty;
    Tavern tavern(playerInventory, playerParty);
    magicStore magicStore(playerInventory);
    NPCGenerator npcGen;
    SpellDatabase spellDB;
    TravelSystem travelSystem(npcGen, debugMode);

    int actionCounter = 0;
    int lastWeekPaid = 0;
    bool running = true;

    while (running) {
        system("cls");

        if (hero.timeSystem.getTotalWeeks() > lastWeekPaid && !playerParty.empty()) {
            deductWeeklyWages(hero, playerParty);
            lastWeekPaid = hero.timeSystem.getTotalWeeks();
        }
        std::vector<MenuItem> items;
        std::map<std::string, std::vector<size_t>> categories;

        categories["Character"] = {};
        items.push_back({"Show Player Stats", "View your character's current stats.", [&]() { heroStats.showStats(); }});
        categories["Character"].push_back(items.size() - 1);
        items.push_back({"Inventory", "Manage your items and equipment.", [&]() { playerInventory.showInventory(hero); }});
        categories["Character"].push_back(items.size() - 1);
        items.push_back({"Party Management", "View and manage your party members.", [&]() { manageParty(playerParty, npcGen, hero); }});
        categories["Character"].push_back(items.size() - 1);
        std::string dictionaryName = hero.hasNewDictionaryEntry ? "Dictionary [!]" : "Dictionary";
        items.push_back({dictionaryName, "Review discovered enemies, locations, weapons, events, and special characters.", [&]() { showDictionary(hero); }});
        categories["Character"].push_back(items.size() - 1);

        bool hasStore = hero.currentLocationType == PeacefulVillage || hero.currentLocationType == PeacefulTown;
        bool hasTavern = hero.currentLocationType == PeacefulTown;
        bool hasMagicStore = hero.currentLocationType == SpellStore;

        if (hasStore || hasTavern || hasMagicStore) {
            categories["Local Establishments"] = {};
            if (hasStore) {
                items.push_back({"Store", "Buy potions and equipment.", [&]() { store.openStore(hero); }});
                categories["Local Establishments"].push_back(items.size() - 1);
            }
            if (hasTavern) {
                items.push_back({"Tavern", "Rest, buy food, hire party members.", [&]() { tavern.openTavern(hero, hero.timeSystem); }});
                categories["Local Establishments"].push_back(items.size() - 1);
            }
            if (hasMagicStore) {
                items.push_back({"Magic Store", "Buy spells.", [&]() { magicStore.openStore(hero); }});
                categories["Local Establishments"].push_back(items.size() - 1);
            }
        }

        categories["Actions"] = {};
        items.push_back({"Explore", "Venture out and face challenges.", [&]() {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);

            if (dist(gen) < 0.8f) {
                Enemy enemy = enemyCtrl.encounterEnemy(1, Terrain);
                CombatScreen combatScreen(hero, playerParty, enemy, hero.timeSystem, npcGen, spellDB);
                combatScreen.startCombat(combat, playerInventory);

                if (hero.stats.hitpoints > 0 && enemy.stats.data.hitpoints <= 0) {
                    enemyCtrl.enemyGoldExpDrop(hero, enemy);
                    heroStats.levelUpChecker();
                }
            } else {
                handleEvent(hero, enemyCtrl, combat, playerInventory, heroStats, npcGen);
            }

            actionCounter++;
            if (actionCounter % 4 == 0) {
                hero.timeSystem.advanceTime(hero);
            }
        }});
        categories["Actions"].push_back(items.size() - 1);

        items.push_back({"Travel", "Move to different locations.", [&]() {
            travelSystem.travel(hero, enemyCtrl, combat, playerInventory, heroStats, hero.timeSystem);
        }});
        categories["Actions"].push_back(items.size() - 1);

        items.push_back({"Pass Time", "Advance time without action.", [&]() {
            actionCounter++;
            if (actionCounter % 4 == 0) {
                hero.timeSystem.advanceTime(hero);
                std::cout << "Time has passed.\n";
                system("cls");
            }
        }});
        categories["Actions"].push_back(items.size() - 1);


        categories["System"] = {};
        items.push_back({"Basics", "Explain the game mechanics.", [&]() {
            system("cls");
            std::cout << "\n=== GAME BASICS ===\n";
            std::cout << "Time System:\n";
            std::cout << "- 4 turns advance the state of the day (Morning -> Afternoon -> Evening -> Night).\n";
            std::cout << "- After Night, a new day begins, and weeks accumulate.\n\n";
            std::cout << "Currency System:\n";
            std::cout << "- 100 Copper = 1 Silver\n";
            std::cout << "- 100 Silver = 1 Gold\n";
            std::cout << "- 100 Gold = 1 Platinum\n\n";
            std::cout << "Sleep Mechanic:\n";
            std::cout << "- If not slept, reduce health by ~5% at the start of each new day.\n";
            std::cout << "- Sleep at the Tavern to restore HP and avoid the penalty.\n\n";
            std::cout << "Other Mechanics:\n";
            std::cout << "- Explore to fight enemies or encounter events.\n";
            std::cout << "- Travel to discover new locations.\n";
            std::cout << "- Manage your party, inventory, and stats.\n";
            std::cout << "- Visit stores, taverns, and magic shops in towns.\n\n";
            std::cout << "Press Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }});
        categories["System"].push_back(items.size() - 1);
        items.push_back({"Exit", "Quit the game.", [&]() { running = false; }});
        categories["System"].push_back(items.size() - 1);

        Menu menu(items, categories, hero);
        menu.displayAndExecute();
        system("cls");
    }
}
