#include "menu.hpp"

#include <iostream>

#include "core/ui.hpp"
#include "core/version.hpp"
#include "app/display.hpp"

void Menu::displayAndExecute() {
    while (true) {
        display();
        int choice = getNumberInput(1, static_cast<int>(items.size()));
        if (choice >= 1 && choice <= static_cast<int>(items.size())) {
            items[static_cast<size_t>(choice - 1)].action();
            clearScreen();
            break;
        } else {
            display::menuInvalidChoice();
        }
    }
}

void Menu::display() {
    std::vector<std::string> lines = {
        std::string("[ RPGGame / v") + gameVersion + " ]",
        hero.name + " / " + hero.playerClass.name + " / Level " + std::to_string(hero.stats.level),
        resourceBar("HP", hero.stats.hitpoints, hero.stats.maxHitpoints),
        resourceBar("MP", hero.stats.mana, hero.stats.maxMana),
        "Current Location: " + hero.currentLocation,
        "Current Time: " + hero.timeSystem.getPeriodString() + " | " + hero.timeSystem.getFormattedTimePassed(),
        "Currency: " + std::to_string(hero.economy.platinum) + "p " +
            std::to_string(hero.economy.gold) + "g " + std::to_string(hero.economy.silver) + "s " +
            std::to_string(hero.economy.copper) + "c"
    };

    const std::vector<std::string> categoryOrder = {"Character", "Local Establishments", "Actions", "System"};
    for (const auto& catName : categoryOrder) {
        auto it = categories.find(catName);
        if (it != categories.end() && !it->second.empty()) {
            lines.push_back("");
            lines.push_back("[" + catName + "]");
            for (size_t idx : it->second) {
                lines.push_back(std::to_string(idx + 1) + ". " + items[idx].name + " - " + items[idx].description);
            }
        }
    }
    displayBorderedMenu(lines, "Choose an option: ");
}
