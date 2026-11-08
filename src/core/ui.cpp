#include "ui.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <sstream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <io.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace {
bool interactiveOutput() {
#ifdef _WIN32
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}

bool styledOutput() {
    const char* term = std::getenv("TERM");
    if (!interactiveOutput() || std::getenv("NO_COLOR") || (term && std::string(term) == "dumb")) return false;
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    return GetConsoleMode(output, &mode) && SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
    return true;
#endif
}

size_t panelWidth() {
    int columns = 80;
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (interactiveOutput() && GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
        columns = info.srWindow.Right - info.srWindow.Left + 1;
#else
    winsize size{};
    if (interactiveOutput() && ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col)
        columns = size.ws_col;
#endif
    return static_cast<size_t>(std::clamp(columns - 2, 20, 92));
}

// Count UTF-8 code points so wrapping never cuts a multibyte character in half.
size_t textWidth(const std::string& text) {
    return std::count_if(text.begin(), text.end(), [](unsigned char c) { return (c & 0xc0) != 0x80; });
}

std::vector<std::string> wrapLine(const std::string& text, size_t width) {
    std::vector<std::string> result;
    std::istringstream paragraphs(text);
    std::string paragraph;
    while (std::getline(paragraphs, paragraph)) {
        std::istringstream words(paragraph);
        std::string word, row;
        while (words >> word) {
            if (!row.empty() && textWidth(row) + 1 + textWidth(word) > width) {
                result.push_back(row);
                row.clear();
            }
            while (textWidth(word) > width) {
                size_t end = 0, count = 0;
                while (end < word.size()) {
                    if ((static_cast<unsigned char>(word[end]) & 0xc0) != 0x80 && count++ == width) break;
                    ++end;
                }
                result.push_back(word.substr(0, end));
                word.erase(0, end);
            }
            if (!row.empty()) row += ' ';
            row += word;
        }
        result.push_back(row);
    }
    if (result.empty()) result.push_back("");
    return result;
}
}

void clearScreen() {
    const char* term = std::getenv("TERM");
    if (!interactiveOutput() || (term && std::string(term) == "dumb")) return;
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void displayBorderedMenu(const std::vector<std::string>& lines, const std::string& prompt) {
    const size_t width = panelWidth() - 4;
    const bool color = styledOutput();
    const std::string border = "+" + std::string(width + 2, '=') + "+";
    auto rule = [&] {
        if (color) std::cout << "\033[2;36m";
        std::cout << border;
        if (color) std::cout << "\033[0m";
        std::cout << '\n';
    };
    // Whole-row tint only: escapes never enter the text, so width math is safe.
    auto tintFor = [](const std::string& row) -> const char* {
        if (row.empty()) return nullptr;
        if (row.front() == '[' || row.front() == '=') return "\033[1;36m";      // headings
        if (row.rfind("---", 0) == 0) return "\033[2;36m";                     // separators
        if (row.find("HP:") != std::string::npos || row.find("HP [") != std::string::npos) return "\033[1;32m";  // health
        if (row.find("MP:") != std::string::npos || row.find("MP [") != std::string::npos ||
            row.find("Mana:") != std::string::npos) return "\033[1;34m";                                      // mana
        if (row.find("Critical") != std::string::npos) return "\033[1;33m";    // crits
        if (row.find("dodged") != std::string::npos) return "\033[1;35m";      // dodges
        if (row.find("Debuffs:") != std::string::npos ||
            row.find("Meters:") != std::string::npos) return "\033[2;31m";     // debuffs
        return nullptr;
    };
    rule();
    for (const auto& line : lines) {
        for (const auto& row : wrapLine(line, width)) {
            std::cout << "| ";
            const char* tint = tintFor(row);
            if (color && tint) std::cout << tint;
            std::cout << row;
            if (color && tint) std::cout << "\033[0m";
            std::cout << std::string(width - textWidth(row), ' ') << " |\n";
        }
    }
    rule();
    if (color && !prompt.empty()) std::cout << "\033[1;33m";
    std::cout << prompt;
    if (color && !prompt.empty()) std::cout << "\033[0m";
    std::cout << std::flush;
}

std::string resourceBar(const std::string& label, int current, int maximum) {
    const int filled = maximum > 0 ? static_cast<int>(12LL * std::clamp(current, 0, maximum) / maximum) : 0;
    return label + " [" + std::string(filled, '#') + std::string(12 - filled, '-') + "] " +
           std::to_string(current) + "/" + std::to_string(maximum);
}

int PagedSelector::select() {
    if (items.empty()) return -1;
    size_t selectedIndex = 0;
    bool choosing = true;

    while (choosing) {
        clearScreen();
        size_t pageEnd = std::min<size_t>(pageStart + pageSize, items.size());
        const size_t pageCount = items.empty() ? 1 : (items.size() + pageSize - 1) / pageSize;
        const size_t pageNumber = pageStart / pageSize + 1;

        std::vector<std::string> lines;
        if (!title.empty()) lines.push_back("[ " + title + " ]");
        lines.push_back("Page " + std::to_string(pageNumber) + " of " + std::to_string(pageCount));
        lines.push_back("----------------------------");
        for (size_t i = pageStart; i < pageEnd; ++i)
            lines.push_back(std::to_string(i + 1) + ". " + items[i]);
        lines.push_back("----------------------------");
        lines.push_back("0. Back");
        if (pageStart > 0) lines.push_back("-1. Previous page");
        if (pageEnd < items.size())
            lines.push_back(std::to_string(items.size() + 1) + ". Next page");
        displayBorderedMenu(lines, "Choose an item or navigation option: ");
        std::string input;
        if (!std::getline(std::cin, input)) return -1;

        int choice = 0;
        try {
            choice = std::stoi(input);
        } catch (...) {
            std::cout << "Invalid input! Enter a number.\n";
            continue;
        }

        if (choice == 0) return -1;
        if (choice < 0) {

            if (pageStart > 0) {
                pageStart = pageStart >= pageSize ? pageStart - pageSize : 0;
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
        if (!std::getline(std::cin, input)) {
            if (std::cin.eof()) {
                std::cout << "\nInput stream closed; exiting.\n";
                std::exit(0);
            }
            std::cin.clear();
            continue;
        }

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
