#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "core/types.hpp"
#include "entities/player.hpp"

// Renders the categorized main menu and dispatches the chosen action.
class Menu {
public:
    Menu(const std::vector<MenuItem>& menuItems, const std::map<std::string, std::vector<size_t>>& menuCategories, Player& heroRef, std::string standingStatus = "")
        : items(menuItems), categories(menuCategories), hero(heroRef), status(std::move(standingStatus)) {}

    void displayAndExecute(std::string* activeCategory = nullptr);

private:
    std::vector<MenuItem> items;
    std::map<std::string, std::vector<size_t>> categories;
    Player& hero;
    std::string status;

    std::vector<std::string> visibleCategories() const;
    std::vector<std::string> statusLines() const;
    void displayOverview(const std::vector<std::string>& order);
    void displayCategory(const std::string& category);
};
