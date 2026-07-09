#include "menu.hpp"

#include <iostream>

#include "core/ui.hpp"

void Menu::displayAndExecute() {
    while (true) {
        display();
        int choice = getNumberInput(1, static_cast<int>(items.size()));
        if (choice >= 1 && choice <= static_cast<int>(items.size())) {
            items[static_cast<size_t>(choice - 1)].action();
            system("cls");
            break;
        } else {
            std::cout << "Invalid choice!" << std::endl;
        }
    }
}

void Menu::display() {

    size_t maxWidth = 0;

    std::string timeLine = "Current Time: " + hero.timeSystem.getPeriodString();
    std::string passedLine = "Time Passed: " + hero.timeSystem.getFormattedTimePassed();
    std::string locationLine = "Current Location: " + hero.currentLocation;
    std::string currencyLine = "Currency: " + std::to_string(hero.economy.platinum) + "p " +
                             std::to_string(hero.economy.gold) + "g " +
                             std::to_string(hero.economy.silver) + "s " +
                             std::to_string(hero.economy.copper) + "c";

    maxWidth = std::max(maxWidth, timeLine.length());
    maxWidth = std::max(maxWidth, passedLine.length());
    maxWidth = std::max(maxWidth, locationLine.length());
    maxWidth = std::max(maxWidth, currencyLine.length());

    for (const auto& cat : categories) {
        if (!cat.second.empty()) {
            std::string catLine = "[" + cat.first + "]";
            maxWidth = std::max(maxWidth, catLine.length());
        }
    }


    for (const auto& item : items) {
        std::string optionLine = std::to_string(&item - &items[0] + 1) + ". " + item.name + " - " + item.description;
        maxWidth = std::max(maxWidth, optionLine.length());
    }

    std::string border = std::string(maxWidth, '=');
    std::string emptyLine = std::string(maxWidth, ' ');

    std::cout << border << std::endl;
    std::cout << timeLine << std::string(maxWidth - timeLine.length(), ' ') << std::endl;
    std::cout << passedLine << std::string(maxWidth - passedLine.length(), ' ') << std::endl;
    std::cout << locationLine << std::string(maxWidth - locationLine.length(), ' ') << std::endl;
    std::cout << currencyLine << std::string(maxWidth - currencyLine.length(), ' ') << std::endl;
    std::cout << emptyLine << std::endl;


    std::vector<std::string> categoryOrder = {"Character", "Local Establishments", "Actions", "System"};

    for (const std::string& catName : categoryOrder) {
        auto it = categories.find(catName);
        if (it != categories.end() && !it->second.empty()) {
            std::string catLine = "[" + catName + "]";
            std::cout << catLine << std::string(maxWidth - catLine.length(), ' ') << std::endl;
            for (size_t idx : it->second) {
                std::string optionLine = std::to_string(idx + 1) + ". " + items[idx].name + " - " + items[idx].description;
                std::cout << optionLine << std::string(maxWidth - optionLine.length(), ' ') << std::endl;
            }
            std::cout << emptyLine << std::endl;
        }
    }

    std::cout << border << std::endl;
    std::cout << "Choose an option: ";
}
