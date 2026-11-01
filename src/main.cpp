#include <iostream>
#include <string>
#include <vector>

#include "core/ui.hpp"
#include "core/version.hpp"
#include "data/class.hpp"
#include "data/race.hpp"
#include "app/main_menu.hpp"
#include "entities/player.hpp"
#include "app/display.hpp"
#include "systems/save.hpp"

int main() {
    while (true) {
        displayBorderedMenu({std::string("[ RPGGame / v") + gameVersion + " ]",
            "Explore. Join a guild. Leave your mark.", "",
            "1. Start", "2. Continue", "3. Exit", "",
            "Enter an option number, then press Enter."}, "Choose an option: ");
        int choice = getNumberInput(1, 3);
        if (choice == 3) return 0;
        if (choice == 1) break;
        std::string error;
        auto session = saves::load(saves::defaultPath, error);
        if (session) {
            mainMenu(*session);
            return 0;
        }
        display::message("Continue failed: " + error);
        clearScreen();
    }

    clearScreen();

    PlayerRaceDatabase raceDb;
    PlayerClassCollection classDb;

    std::vector<std::string> raceNames;
    for (const auto& r : raceDb.templates) raceNames.push_back(std::to_string(raceNames.size() + 1) + ". " + r.name);
    int raceChoice;
    do {
        clearScreen();
        displayBorderedMenu(raceNames, "Choose your race: ");
        raceChoice = getNumberInput(1, static_cast<int>(raceNames.size()));
        clearScreen();
        display::racePreview(raceDb.templates[raceChoice - 1]);
        displayBorderedMenu({"1. Confirm race", "2. Back"}, "Choose an option: ");
    } while (getNumberInput(1, 2) != 1);
    const PlayerRaceTemplate& chosenRace = raceDb.templates[raceChoice - 1];
    clearScreen();

    std::vector<std::string> classNames;
    for (const auto& c : classDb.templates) classNames.push_back(std::to_string(classNames.size() + 1) + ". " + c.name);
    int classChoice;
    do {
        displayBorderedMenu(classNames, "Choose your class: ");
        classChoice = getNumberInput(1, static_cast<int>(classNames.size()));
        clearScreen();
        display::classPreview(classDb.templates[classChoice - 1], chosenRace);
        displayBorderedMenu({"1. Confirm class", "2. Back"}, "Choose an option: ");
    } while (getNumberInput(1, 2) != 1);
    const PlayerClassTemplate& chosenClass = classDb.templates[classChoice - 1];
    clearScreen();

    std::string name = display::characterName();
    clearScreen();

    PlayerRace playerRace{ chosenRace.name, { chosenRace.lore.description } };
    Player hero(name, playerRace, chosenClass);
    hero.applyRaceBonus(chosenRace.statBonus);

    display::characterCreated();
    clearScreen();
    GameSession session(hero);
    mainMenu(session);
    return 0;
}
