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
    const std::string border = "+-" + std::string(width, '-') + "-+";
    auto rule = [&] {
        if (color) std::cout << "\033[2;36m";
        std::cout << border;
        if (color) std::cout << "\033[0m";
        std::cout << '\n';
    };
    rule();
    for (const auto& line : lines) {
        for (const auto& row : wrapLine(line, width)) {
            std::cout << "| ";
            const bool heading = !row.empty() && (row.front() == '[' || row.front() == '=');
            if (color && heading) std::cout << "\033[1;36m";
            std::cout << row;
            if (color && heading) std::cout << "\033[0m";
            std::cout << std::string(width - textWidth(row), ' ') << " |\n";
        }
    }
    rule();
    if (color && !prompt.empty()) std::cout << "\033[1m";
    std::cout << prompt;
    if (color && !prompt.empty()) std::cout << "\033[0m";
    std::cout << std::flush;
}

std::string resourceBar(const std::string& label, int current, int maximum) {
    const int filled = maximum > 0 ? static_cast<int>(12LL * std::clamp(current, 0, maximum) / maximum) : 0;
    return label + " [" + std::string(filled, '#') + std::string(12 - filled, '-') + "] " +
           std::to_string(current) + "/" + std::to_string(maximum);
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
