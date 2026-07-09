#include "ui.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <thread>

void displayBorderedMenu(const std::vector<std::string>& lines, const std::string& prompt) {
    size_t maxWidth = 0;
    for (const auto& line : lines) {
        maxWidth = std::max(maxWidth, line.length());
    }
    std::string border = std::string(maxWidth, '=');
    std::cout << border << std::endl;
    for (const auto& line : lines) {
        std::cout << line << std::string(maxWidth - line.length(), ' ') << std::endl;
    }
    std::cout << border << std::endl;
    std::cout << prompt;
}

size_t PagedSelector::select() {
    size_t selectedIndex = 0;
    bool choosing = true;

    while (choosing) {
        size_t pageEnd = std::min<size_t>(pageStart + pageSize, items.size());

        for (size_t i = pageStart; i < pageEnd; ++i) {
            std::cout << i + 1 << ". " << items[i] << std::endl;
        }

        std::cout << "Enter number to select, or input a number out of range to scroll (negative for prev, >" << items.size() << " for next):" << std::endl;
        std::string input;
        std::getline(std::cin, input);

        int choice = 0;
        try {
            choice = std::stoi(input);
        } catch (...) {
            std::cout << "Invalid input! Enter a number.\n";
            continue;
        }

        if (choice < 1) {

            if (pageStart > 0) {
                pageStart = std::max<size_t>(0, pageStart - pageSize);
            } else {
                std::cout << "Already on first page.\n";
            }
        } else if (choice > static_cast<int>(items.size())) {

            if (pageStart + pageSize < items.size()) {
                pageStart += pageSize;
            } else {
                std::cout << "Already on last page.\n";
            }
        } else if (choice > static_cast<int>(pageEnd) || choice <= static_cast<int>(pageStart)) {

            pageStart = (static_cast<size_t>(choice) - 1) / pageSize * pageSize;
        } else {
            selectedIndex = static_cast<size_t>(choice) - 1;
            choosing = false;
        }
    }

    return selectedIndex;
}

int getNumberInput(int min, int max) {
    while (true) {
        std::string input;
        std::getline(std::cin, input);

        try {
            int choice = std::stoi(input);
            if (choice >= min && choice <= max)
                return choice;
            else
                std::cout << "Please enter a number between " << min << " and " << max << ": ";
        } catch (...) {
            std::cout << "Invalid input! Enter a number: ";
        }
    }
}

void narrate(const std::string& text, int delay) {
    for (char c : text) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}
