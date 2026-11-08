#include "player_controller.hpp"

#include "app/display.hpp"
#include "core/ui.hpp"

void PlayerController::showStats() const {
    clearScreen();
    display::showPlayerStats(player, reqAmount, inventory.getActiveBuff());
}

void PlayerController::levelUpChecker() {
    while (player.stats.expe >= reqAmount) {
        player.stats.expe -= reqAmount;
        player.stats.level += 1;


        player.stats.scale(1.1f);


        reqAmount *= 1.2f;

        display::leveledUp();
    }
}
