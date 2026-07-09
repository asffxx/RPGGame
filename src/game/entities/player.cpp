#include "player.hpp"

void Player::applyRaceBonus(const Stats& bonus) {
    stats.hitpoints += bonus.hitpoints;
    stats.maxHitpoints += bonus.maxHitpoints;
    stats.armor += bonus.armor;
    stats.magicArmor += bonus.magicArmor;
    stats.attack += bonus.attack;
    stats.magicAttack += bonus.magicAttack;
    stats.mana += bonus.mana;
    stats.maxMana += bonus.maxMana;
}

void Player::scaleStats(float multiplier) {
    stats.scale(multiplier);
}
