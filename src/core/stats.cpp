#include "stats.hpp"

void Stats::scale(float multiplier) {
    maxHitpoints = static_cast<int>(std::round(maxHitpoints * multiplier));
    maxMana = static_cast<int>(std::round(maxMana * multiplier));
    hitpoints = static_cast<int>(std::round(hitpoints * multiplier));
    hitpoints = std::min<int>(hitpoints, maxHitpoints);
    mana = static_cast<int>(std::round(mana * multiplier));
    mana = std::min<int>(mana, maxMana);
    armor *= multiplier;
    magicArmor *= multiplier;
    attack = static_cast<int>(std::round(attack * multiplier));
    magicAttack = static_cast<int>(std::round(magicAttack * multiplier));
    dodgeRate *= multiplier;
    critRate *= multiplier;
    critDamage *= multiplier;
    magicAmplifierRate *= multiplier;
}
