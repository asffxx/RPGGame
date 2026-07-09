#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "core/ui.hpp"
#include "db/class.hpp"
#include "db/race.hpp"
#include "game/app/main_menu.hpp"
#include "game/entities/player.hpp"

int main() {
    PlayerRaceDatabase raceDb;
    PlayerClassCollection classDb;

    narrate("\n--- SELECT YOUR ANCESTRY ---\n");
    std::vector<std::string> raceNames;
    for (const auto& r : raceDb.templates) raceNames.push_back(r.name);

    PagedSelector raceSelector(raceNames);
    size_t raceIndex = raceSelector.select();
    const PlayerRaceTemplate& chosenRace = raceDb.templates[raceIndex];
    system("cls");

    std::string name;
    narrate("\nWhat is your name? : ");
    std::getline(std::cin, name);
    system("cls");

    narrate(chosenRace.lore.description, 10);

    std::cout << "\nAccept this fate? (y/n): ";
    char confirm;
    std::cin >> confirm;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (confirm == 'n' || confirm == 'N') {
        narrate("Then perhaps it is better to remain in the nothingness.");
        return 0;
    }

    system("cls");
    narrate("\n--- CHOOSE YOUR CALLING ---\n");
    std::vector<std::string> classNames;
    for (const auto& c : classDb.templates) classNames.push_back(c.name);

    PagedSelector classSelector(classNames);
    size_t classIndex = classSelector.select();
    const PlayerClassTemplate& chosenClass = classDb.templates[classIndex];

    PlayerRace playerRace{ chosenRace.name, { chosenRace.lore.description } };
    Player hero(name, playerRace, chosenClass);
    hero.applyRaceBonus(chosenRace.statBonus);

    std::cout << "\nCharacter created successfully!" << std::endl;
    system("cls");
    std::cout << "\nPress Enter to step into the grey...\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    system("cls");
    mainMenu(hero);
    return 0;
}
