#pragma once

#include <string>
#include <vector>

// Clears the terminal screen. Uses `cls` on Windows and `clear` elsewhere, so the
// same source builds and runs on both without 28 separate #ifdefs.
void clearScreen();

// Prints a bordered, fixed-width menu of lines and prompts for input.
void displayBorderedMenu(const std::vector<std::string>& lines, const std::string& prompt = "Choose an option: ");

// Shared resource display, with clamped fill and exact numeric values.
std::string resourceBar(const std::string& label, int current, int maximum);

// Scrollable numbered selector over a list of items.
class PagedSelector {
public:
    PagedSelector(const std::vector<std::string>& itemList, size_t pageSizeParam = 5,
                  std::string title = "")
        : items(itemList), pageSize(pageSizeParam), title(std::move(title)), pageStart(0) {}

    // Returns -1 on Back, empty list, or end of input.
    int select();

private:
    std::vector<std::string> items;
    size_t pageSize;
    std::string title;
    size_t pageStart;
};

// Reads an integer from stdin constrained to [min, max].
int getNumberInput(int min, int max);

// Types out text character-by-character with a delay (milliseconds).
void narrate(const std::string& text, int delay = 40);
