#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "core/ui.hpp"

int main() {
    try {
        std::ostringstream output;
        auto* original = std::cout.rdbuf(output.rdbuf());
        displayBorderedMenu({"[Journal]", "First line\nSecond line", std::string(190, 'x'),
            std::string(73, 'a') + "\xc3\xa9" + "z", "", "1. Back"}, "Choose: ");
        std::cout.rdbuf(original);
        const auto text = output.str();
        if (text.find('\033') != std::string::npos) throw std::runtime_error("ANSI escapes in plain output");
        if (text.find("First line") == std::string::npos || text.find("Second line") == std::string::npos ||
            text.find("\xc3\xa9") == std::string::npos || text.find("1. Back") == std::string::npos)
            throw std::runtime_error("Wrapped content missing or UTF-8 split");
        std::istringstream rows(text);
        std::string row;
        size_t xCount = 0;
        while (std::getline(rows, row)) {
            size_t width = 0;
            for (unsigned char c : row) {
                if ((c & 0xc0) != 0x80) ++width;
                if (c == 'x') ++xCount;
            }
            if (row != "Choose: " && (width != 78 || (row.front() != '+' && row.front() != '|')))
                throw std::runtime_error("Panel rows misaligned or too wide");
        }
        if (xCount != 190) throw std::runtime_error("Long unbroken text was truncated");
        if (resourceBar("MP", 0, 0) != "MP [------------] 0/0" ||
            resourceBar("HP", 5, 10) != "HP [######------] 5/10" ||
            resourceBar("HP", -1, 10) != "HP [------------] -1/10" ||
            resourceBar("HP", 20, 10) != "HP [############] 20/10")
            throw std::runtime_error("Resource fill or numeric value incorrect");
        std::cout << "Passed terminal panels and resource bars\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
