#include "debuff.hpp"

DebuffDatabase::DebuffDatabase() {
    debuffList = {

         {"Burning",
        {10, 0, 0, 0, 0, 3, 5, 0, 1.0f},   // healthDecrease, attackDecrease, defenseDecrease, magicAttackDecrease, magicDefenseDecrease, durationTurns, damageProc, addedTurnOnTrigger, amplifierOnTrigger
        {true, 5, "Scorched"},              // stackable, maxStacks, maxStackEffectName
        {true, true, "Fire Aspect", 0.3f}, // throwable, enchantOnWeapon, nameOnEnchantment, effectChance
        "Deals 10 damage over 3 turns."
    },


        {"Freeze",
        {0, 0, 0, 0, 0, 3, 0, 2, 1.2f},
        {true, 6, "Frostbite"},
        {true, true, "Frost Aspect", 1.0f},
        "Freezes the target, slowing them and dealing frost damage over time."
    },


        {"Poison",
        {5, 0, 0, 0, 0, 3, 0, 2, 1.3f},
        {true, 6, "Envenomed"},
        {true, true, "Poisoned", 1.0f},
        "Poisons the target, dealing damage over time and stacking poison effects."
    },


        {"Bleeding",
        {5, 0, 0, 0, 0, 0, 0, 0, 2.0f},
        {true, 8, "Hemorrhage"},
        {true, true, "Vampiric", 1.0f},
        "Causes severe bleeding over time, dealing high damage and may heal the attacker."
    },


        {"Weakness",
        {0, 5, 0, 0, 0, 3, 0, 0, 1.0f},
        {true, 3, "Enfeebled"},
        {true, true, "Weakening Strike", 1.0f},
        "Reduces the target's attack temporarily."
    },

        {"Vulnerability",
        {0, 0, 0, 0, 0, 3, 0, 0, 1.0f},
        {true, 3, "Exposed"},
        {true, true, "Crippling Strike", 1.0f},
        "Makes the target more vulnerable, increasing damage taken."
    },

        {"Magic Dampening",
        {0, 0, 0, 5, 0, 3, 0, 0, 1.0f},
        {true, 3, "Arcane Suppressed"},
        {true, true, "Nullifying Strike", 1.0f},
        "Reduces the target's magic damage output."
    },

        {"Mana Weakness",
        {0, 0, 0, 0, 0, 3, 10, 0, 1.0f},
        {true, 3, "Mana Starved"},
        {true, true, "Mana Drain", 1.0f},
        "Reduces the target's mana and ability to cast spells."
    },

        {"Health Blight",
        {0, 0, 0, 0, 0, 3, 0, 0, 1.5f},
        {true, 3, "Blighted"},
        {true, true, "Blight Enchantment", 1.0f},
        "Reduces the target's max HP and increases all incoming damage."
    }
    };
}
