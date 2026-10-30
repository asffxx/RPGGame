#include <iostream>
#include <string>
#include <vector>

#include "core/ui.hpp"
#include "data/class.hpp"
#include "data/race.hpp"
#include "app/main_menu.hpp"
#include "entities/player.hpp"
#include "app/display.hpp"

int main() {
    std::vector<std::string> titleOptions = {"Start", "Exit"};
    displayBorderedMenu(titleOptions, "Choose an option: ");
    int choice = getNumberInput(1, 2);
    if (choice != 1) return 0;

    clearScreen();

    PlayerRaceDatabase raceDb;
    PlayerClassCollection classDb;

    std::vector<std::string> raceNames;
    for (const auto& r : raceDb.templates) raceNames.push_back(r.name);
    displayBorderedMenu(raceNames, "Choose your race: ");
    int raceChoice = getNumberInput(1, static_cast<int>(raceNames.size()));
    const PlayerRaceTemplate& chosenRace = raceDb.templates[raceChoice - 1];
    clearScreen();

    std::vector<std::string> classNames;
    for (const auto& c : classDb.templates) classNames.push_back(c.name);
    displayBorderedMenu(classNames, "Choose your class: ");
    int classChoice = getNumberInput(1, static_cast<int>(classNames.size()));
    const PlayerClassTemplate& chosenClass = classDb.templates[classChoice - 1];
    clearScreen();

    std::cout << "Enter your name: ";
    std::string name;
    std::getline(std::cin, name);
    clearScreen();

    PlayerRace playerRace{ chosenRace.name, { chosenRace.lore.description } };
    Player hero(name, playerRace, chosenClass);
    hero.applyRaceBonus(chosenRace.statBonus);

    display::characterCreated();
    clearScreen();
    mainMenu(hero);
    return 0;
}
