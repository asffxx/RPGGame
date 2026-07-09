#pragma once

#include <cmath>

// Core stat block shared by players, NPCs, and enemies.
struct Stats {
    int hitpoints;
    int maxHitpoints;
    float armor;
    float magicArmor;
    int attack;
    int magicAttack;
    int mana;
    int maxMana;
    float critRate;
    float critDamage;
    float dodgeRate;
    float magicAmplifierRate;
    int level;
    float expe;

    void scale(float multiplier);
};
