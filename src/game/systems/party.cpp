#include "party.hpp"

#include <iostream>
#include <limits>
#include <random>

#include "core/ui.hpp"

void deductWeeklyWages(Player& player, std::vector<NPC>& playerParty) {
    int totalWages = 0;
    for (const auto& npc : playerParty) {
        totalWages += npc.wagePerWeek;
    }
    if (player.economy.subtractCurrency(0, totalWages, 0, 0)) {
        std::cout << "Paid " << totalWages << " gold in wages to party members.\n";
    } else {
        std::cout << "Not enough gold to pay wages! Party members may become unhappy.\n";
    }
}

void manageParty(std::vector<NPC>& playerParty, NPCGenerator& npcGen, Player& player) {
    if (playerParty.empty()) {
        std::cout << "Your party is empty.\n";
        std::cout << "Press Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }


    int totalWages = 0;
    for (const auto& npc : playerParty) {
        totalWages += npc.wagePerWeek;
    }
    if (!player.economy.subtractCurrency(0, totalWages, 0, 0)) {

        static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, playerParty.size() - 1);
        size_t index = dist(gen);
        std::string name = playerParty[index].name;
        npcGen.unlockName(name);
        playerParty.erase(playerParty.begin() + static_cast<int>(index));
        std::cout << "Due to insufficient gold, " << name << " has left the party.\n";
        std::cout << "Press Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    } else {

        player.economy.addCurrency(0, totalWages, 0, 0);
    }

    std::vector<std::string> lines;
    lines.push_back("Current Party Members:");
    for (size_t i = 0; i < playerParty.size(); ++i) {
        const auto& npc = playerParty[i];
        std::string line = std::to_string(i + 1) + ". " + npc.name + " (" + npc.race.name + " " + npc.playerClass.name + ", Level " + std::to_string(npc.level) + ") - Weekly Wage: " + std::to_string(npc.wagePerWeek) + " gold";
        lines.push_back(line);
    }
    lines.push_back("Select a member (0 to cancel):");
    displayBorderedMenu(lines, "");

    int choice = getNumberInput(0, static_cast<int>(playerParty.size()));
    if (choice == 0) return;

    size_t index = static_cast<size_t>(choice - 1);

    std::cout << "What do you want to do with " << playerParty[index].name << "?" << std::endl;
    std::cout << "1. Talk to member" << std::endl;
    std::cout << "2. What is your story?" << std::endl;
    std::cout << "3. Kick member" << std::endl;
    std::cout << "4. Cancel" << std::endl;
    int action = getNumberInput(1, 4);

    if (action == 1) {

        if (!playerParty[index].dialogues.empty()) {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<size_t> dist(0, playerParty[index].dialogues.size() - 1);
            std::cout << playerParty[index].name << " says: \"" << playerParty[index].dialogues[dist(gen)] << "\"" << std::endl;
        } else {
            std::cout << playerParty[index].name << " has nothing to say." << std::endl;
        }
        std::cout << "Press Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (action == 2) {
        std::cout << playerParty[index].name << "'s story: " << playerParty[index].story << std::endl;
        std::cout << "Press Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (action == 3) {

        std::string name = playerParty[index].name;
        npcGen.unlockName(name);
        playerParty.erase(playerParty.begin() + static_cast<int>(index));
        std::cout << name << " has been kicked from the party.\n";
        std::cout << "Press Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else {
        // Cancel
    }
}
