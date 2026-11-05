#pragma once

#include <string>
#include <vector>
#include <functional>

// Where the player (or an enemy) can be.
enum LocationType { PeacefulVillage, PeacefulTown, Dungeon, Terrain, SpellStore, PeacefulCity };

// Settings describing how a debuff behaves.
namespace debuffSettings {
    struct DebuffEffect {
        int healthDecrease = 0;   // HP lost per stack per combat round
        int manaDecrease = 0;     // Mana lost per stack per combat round
        int attackDecrease = 0;   // Flat attack reduction per stack
        int defenseDecrease = 0;  // Armor reduction in percentage points per stack
        int magicAttackDecrease = 0;
        int magicDefenseDecrease = 0;

        int durationTurns = 1;
        int addedTurnOnTrigger = 0;
        float amplifierOnTrigger = 1.0f; // damage multiplier at maximum stacks
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

// A debuff currently applied to a combatant. Duration counts combat rounds.
struct ActiveDebuff {
    std::string name;
    int remainingTurns = 0;
    int stacks = 1;
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
