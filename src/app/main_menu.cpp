#include "main_menu.hpp"

#include <iostream>
#include <map>
#include <random>
#include <vector>

#include "core/types.hpp"
#include "core/ui.hpp"
#include "data/spell.hpp"
#include "entities/npc.hpp"
#include "combat/combat.hpp"
#include "systems/dictionary.hpp"
#include "entities/enemy.hpp"
#include "world/establishments.hpp"
#include "systems/event.hpp"
#include "systems/inventory.hpp"
#include "ui/menu.hpp"
#include "systems/party.hpp"
#include "ui/player_controller.hpp"
#include "world/travel.hpp"
#include "app/display.hpp"
#include "systems/save.hpp"
#include "world/encounters.hpp"
#include "systems/guild.hpp"

void mainMenu(GameSession& session, bool debugMode) {
    auto& hero = session.hero;
    EnemyController enemyCtrl;
    CombatSystem combat;
    auto& playerInventory = session.inventory;
    PlayerController heroStats(hero, playerInventory);
    Store store(playerInventory);
    auto& playerParty = session.party;
    Tavern tavern(playerInventory, playerParty, session);
    magicStore magicStore(playerInventory);
    NPCGenerator npcGen;
    for (const auto& npc : playerParty) npcGen.lockName(npc.name);
    SpellDatabase spellDB;
    TravelSystem travelSystem(npcGen, session, debugMode);

    auto& actionCounter = session.actionCounter;
    auto& lastWeekPaid = session.lastWeekPaid;
    bool running = true;

    while (running) {
        clearScreen();

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
        items.push_back({"Quest Journal", "Review quests, commissions, and city leads.", [&]() { showQuestJournal(session); }});
        categories["Character"].push_back(items.size() - 1);

        bool isCity = hero.currentLocationType == PeacefulCity;
        bool hasStore = hero.currentLocationType == PeacefulVillage || hero.currentLocationType == PeacefulTown || isCity;
        bool hasTavern = hero.currentLocationType == PeacefulTown || isCity;
        bool hasMagicStore = hero.currentLocationType == SpellStore || isCity;

        if (hasStore || hasTavern || hasMagicStore) {
            categories["Local Establishments"] = {};
            if (hasStore) {
                items.push_back({"Store", "Buy potions and equipment.", [&]() { store.openStore(hero); }});
                categories["Local Establishments"].push_back(items.size() - 1);
            }
            if (hasTavern) {
                items.push_back({"Tavern", "Rest, buy food, hire companions, or hear rumors.", [&]() { tavern.openTavern(hero, hero.timeSystem); }});
                categories["Local Establishments"].push_back(items.size() - 1);
            }
            if (hasMagicStore) {
                items.push_back({"Magic Store", "Buy spells.", [&]() { magicStore.openStore(hero); }});
                categories["Local Establishments"].push_back(items.size() - 1);
            }
            if (isCity && guildDatabase().inCity(hero.currentLocation)) {
                items.push_back({"Guild Hall", "Join the city's guild and undertake its quests.", [&]() { openGuildHall(session); }});
                categories["Local Establishments"].push_back(items.size() - 1);
            }
        }

        categories["Actions"] = {};
        items.push_back({"Explore", "Venture out and face challenges.", [&]() {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);

            const auto area = currentArea(hero.currentLocation);
            bool eligible = area.type != PeacefulCity && enemyCtrl.hasEligibleEnemy(area.difficultyLevel, area.type);
            if (eligible && dist(gen) < encounterChance(area.type)) {
                Enemy enemy = enemyCtrl.encounterEnemy(area.difficultyLevel, area.type);
                CombatScreen combatScreen(hero, playerParty, enemy, hero.timeSystem, npcGen, spellDB);
                combatScreen.startCombat(combat, playerInventory);

                if (hero.stats.hitpoints > 0 && enemy.stats.data.hitpoints <= 0) {
                    enemyCtrl.enemyGoldExpDrop(hero, enemy);
                    heroStats.levelUpChecker();
                }
            } else {
                handleEvent(hero, enemyCtrl, combat, playerInventory, heroStats, npcGen, playerParty, eligible);
            }

            actionCounter = (actionCounter + 1) % 4;
            if (actionCounter == 0) {
                hero.timeSystem.advanceTime(hero);
            }
        }});
        categories["Actions"].push_back(items.size() - 1);

        if (!questsAtLocation(session).empty()) {
            items.push_back({"Quest Action", "Investigate or confront your quest target here.", [&]() {
                performQuestAction(session, enemyCtrl, combat, npcGen);
            }});
            categories["Actions"].push_back(items.size() - 1);
        }

        items.push_back({"Travel", "Move to different locations.", [&]() {
            travelSystem.travel(hero, enemyCtrl, combat, playerInventory, heroStats, hero.timeSystem);
        }});
        categories["Actions"].push_back(items.size() - 1);

        items.push_back({"Pass Time", "Advance time without action.", [&]() {
            actionCounter = (actionCounter + 1) % 4;
            if (actionCounter == 0) {
                hero.timeSystem.advanceTime(hero);
                display::timePassed();
                clearScreen();
            }
        }});
        categories["Actions"].push_back(items.size() - 1);


        categories["System"] = {};
        items.push_back({"Basics", "Explain the game mechanics.", [&]() {
            clearScreen();
            display::gameBasics();
        }});
        categories["System"].push_back(items.size() - 1);
        items.push_back({"Save Game", "Save this session to savegame.dat (overwrites the slot).", [&]() {
            std::string error;
            bool saved = saves::save(session, saves::defaultPath, error);
            display::message(saved ? "Game saved to savegame.dat." : "Save failed: " + error);
        }});
        categories["System"].push_back(items.size() - 1);
        items.push_back({"Exit", "Quit the game.", [&]() { running = false; }});
        categories["System"].push_back(items.size() - 1);

        Menu menu(items, categories, hero);
        menu.displayAndExecute();
        clearScreen();
    }
}
