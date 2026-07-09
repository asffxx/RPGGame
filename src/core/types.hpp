#pragma once

#include <string>
#include <vector>
#include <functional>

// Where the player (or an enemy) can be.
enum LocationType { PeacefulVillage, PeacefulTown, Dungeon, Terrain, SpellStore };

// Settings describing how a debuff behaves.
namespace debuffSettings {
    struct DebuffEffect {
        int healthDecrease = 0;
        int attackDecrease = 0;
        int defenseDecrease = 0;
        int magicAttackDecrease = 0;
        int magicDefenseDecrease = 0;

        int durationTurns = 0;
        int damageProc = 0;
        int addedTurnOnTrigger = 0;
        float amplifierOnTrigger = 1.0f;
    };

    struct DebuffStackRules {
        bool stackable = false;
        int maxStacks = 1;
        std::string maxStackEffectName;
    };

    struct DebuffApplication {
        bool throwable = false;
        bool enchantOnWeapon = false;
        std::string nameOnEnchantment;
        float effectChance = 1.0f;
    };
}

// A debuff currently applied to a combatant.
struct ActiveDebuff {
    std::string name;
    int remainingDuration;
    int currentStacks;
};

// Outcome of a single attack in combat.
struct CombatResult {
    std::string attackerName;
    std::string targetName;
    int physicalDamage;
    int magicalDamage;
    int totalDamage;
    bool isCrit;
    bool dodge;
    std::string debuffInflicted;
};

// A single selectable entry in the main menu.
struct MenuItem {
    std::string name;
    std::string description;
    std::function<void()> action;
};
