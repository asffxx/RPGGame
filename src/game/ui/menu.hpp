#pragma once

#include <map>
#include <string>
#include <vector>

#include "core/types.hpp"
#include "game/entities/player.hpp"

// Renders the categorized main menu and dispatches the chosen action.
class Menu {
public:
    Menu(const std::vector<MenuItem>& menuItems, const std::map<std::string, std::vector<size_t>>& menuCategories, Player& heroRef)
        : items(menuItems), categories(menuCategories), hero(heroRef) {}

    void displayAndExecute();

private:
    std::vector<MenuItem> items;
    std::map<std::string, std::vector<size_t>> categories;
    Player& hero;

    void display();
};
