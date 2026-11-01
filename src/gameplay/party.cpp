#include "party.hpp"

#include <iostream>
#include <limits>
#include <random>

#include "core/ui.hpp"
#include "app/display.hpp"

void deductWeeklyWages(Player& player, std::vector<NPC>& playerParty) {
    int totalWages = 0;
    for (const auto& npc : playerParty) {
        totalWages += npc.wagePerWeek;
    }
    if (player.economy.subtractCurrency(0, totalWages, 0, 0)) {
        display::wagesPaid(totalWages);
    } else {
        display::wagesInsufficient();
    }
}

void manageParty(std::vector<NPC>& playerParty, NPCGenerator& npcGen, Player& player) {
    if (playerParty.empty()) {
        display::partyEmpty();
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
        display::partyMemberLeft(name);
        return;
    } else {

        player.economy.addCurrency(0, totalWages, 0, 0);
    }

    display::partyMemberList(playerParty);

    int choice = getNumberInput(0, static_cast<int>(playerParty.size()));
    if (choice == 0) return;

    size_t index = static_cast<size_t>(choice - 1);

    display::partyActionPrompt(playerParty[index].name);
    int action = getNumberInput(1, 4);

    if (action == 1) {

        if (!playerParty[index].dialogues.empty()) {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<size_t> dist(0, playerParty[index].dialogues.size() - 1);
            display::partyDialogue(playerParty[index].name, playerParty[index].dialogues[dist(gen)]);
        } else {
            display::partyDialogue(playerParty[index].name, "has nothing to say.");
        }
    } else if (action == 2) {
        display::partyStory(playerParty[index].name, playerParty[index].story);
    } else if (action == 3) {

        std::string name = playerParty[index].name;
        npcGen.unlockName(name);
        playerParty.erase(playerParty.begin() + static_cast<int>(index));
        display::partyKicked(name);
    } else {
        // Cancel
    }
}
