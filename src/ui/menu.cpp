#include "menu.hpp"

#include "core/ui.hpp"
#include "core/version.hpp"

void Menu::displayAndExecute() {
    const auto order = visibleCategories();
    if (order.empty()) return;

    while (true) {
        displayOverview(order);
        const auto& category = order[static_cast<size_t>(getNumberInput(1, static_cast<int>(order.size())) - 1)];
        const auto& indices = categories.at(category);

        bool acted = false;
        while (!acted) {
            displayCategory(category);
            int choice = getNumberInput(0, static_cast<int>(indices.size()));
            if (choice == 0) break;
            items[indices[static_cast<size_t>(choice - 1)]].action();
            clearScreen();
            acted = true;
        }
        if (acted) return;
    }
}

std::vector<std::string> Menu::visibleCategories() const {
    std::vector<std::string> result;
    for (const auto& name : {"Character", "Local Establishments", "Actions", "System"}) {
        auto found = categories.find(name);
        if (found != categories.end() && !found->second.empty()) result.push_back(name);
    }
    return result;
}

std::vector<std::string> Menu::statusLines() const {
    return {
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
}

void Menu::displayOverview(const std::vector<std::string>& order) {
    auto lines = statusLines();
    lines.push_back("");
    lines.push_back("[ Main Menu ]");
    for (size_t i = 0; i < order.size(); ++i)
        lines.push_back(std::to_string(i + 1) + ". " + order[i] + " (" + std::to_string(categories.at(order[i]).size()) + ")");
    lines.push_back("");
    lines.push_back("Choose a category, then an action.");
    displayBorderedMenu(lines, "Choose a category: ");
}

void Menu::displayCategory(const std::string& category) {
    auto lines = statusLines();
    lines.push_back("");
    lines.push_back("[" + category + "]");
    const auto& indices = categories.at(category);
    for (size_t i = 0; i < indices.size(); ++i) {
        const auto& item = items[indices[i]];
        lines.push_back(std::to_string(i + 1) + ". " + item.name + " - " + item.description);
    }
    lines.push_back("0. Back");
    displayBorderedMenu(lines, "Choose an option: ");
}
