#include "player_controller.hpp"

#include <limits>

void PlayerController::showStats() const {
    std::cout << "\n=== Player Stats ===" << std::endl;
    std::cout << "Hitpoints: " << player.stats.hitpoints << "/" << player.stats.maxHitpoints << std::endl;

    auto activeBuff = inventory.getActiveBuff();
    if (activeBuff) {
        std::cout << "Armor: " << player.stats.armor
             << "( + " << activeBuff->defenseBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << std::endl;
        std::cout << "Magic Armor: " << player.stats.magicArmor
             << "( + " << activeBuff->magicDefenseBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << std::endl;
        std::cout << "Attack: " << player.stats.attack
             << "( + " << activeBuff->attackBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << std::endl;
        std::cout << "Magic Attack: " << player.stats.magicAttack
             << "( + " << activeBuff->magicAttackBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << std::endl;
    } else {
        std::cout << "Armor: " << player.stats.armor << std::endl;
        std::cout << "Magic Armor: " << player.stats.magicArmor << std::endl;
        std::cout << "Attack: " << player.stats.attack << std::endl;
        std::cout << "Magic Attack: " << player.stats.magicAttack << std::endl;
    }

    std::cout << "Level: " << player.stats.level << std::endl;
    std::cout << "Experience: " << player.stats.expe << "/" << reqAmount << std::endl;
    std::cout << "Currency: " << player.economy.platinum << "p " << player.economy.gold << "g " << player.economy.silver << "s " << player.economy.copper << "c" << std::endl;
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void PlayerController::levelUpChecker() {
    while (player.stats.expe >= reqAmount) {
        player.stats.expe -= reqAmount;
        player.stats.level += 1;


        player.stats.scale(1.1f);


        reqAmount *= 1.2f;

        std::cout << "You leveled Up!" << std::endl;
        std::cout << "Your stats have been increased by 10%." << std::endl;
    }
}
