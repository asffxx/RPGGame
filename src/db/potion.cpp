#include "potion.hpp"

PotionDatabase::PotionDatabase() {
    potionList = {
        { "Small Health Potion", 50, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0, "Restores 50 HP", 0, 0, 12, 0 },
        { "Large Health Potion", 150, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0, "Restores 150 HP", 0, 0, 36, 0 },
        { "Extra Large Health Potion", 300, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0, "Restores 300 HP", 0, 0, 72, 0 },
        { "Small Mana Potion", 0, 0, 0, 0, 15, 5, 0, 0.0f, 0.0f, 3, "Restores 15 Mana for 3 turns", 0, 0, 24, 0 },
        { "Large Mana Potion", 0, 0, 0, 0, 40, 10, 0, 0.0f, 0.0f, 3, "Restores 40 Mana for 3 turns", 0, 0, 60, 0 },
        { "Extra Large Mana Potion", 0, 0, 0, 0, 80, 20, 0, 0.0f, 0.0f, 3, "Restores 80 Mana for 3 turns", 0, 0, 120, 0 },
        { "Strength Potion", 0, 10, 0, 0, 0, 0, 0, 0.0f, 0.0f, 3, "Increases Attack by 10 for 3 turns", 0, 0, 30, 0 },
        { "Defense Potion", 0, 0, 5, 0, 0, 0, 0, 0.0f, 0.0f, 3, "Increases Defense by 5 for 3 turns", 0, 0, 30, 0 },
        { "Magic Elixir", 0, 0, 0, 15, 20, 10, 0, 0.0f, 0.0f, 3, "Increases Magic Attack by 15 and restores 20 Mana for 3 turns", 0, 0, 48, 0 },
        { "Rage Potion", 0, 0, 0, 0, 0, 0, 0, 0.1f, 0.5f, 3, "Increases Crit Rate by 10% and Crit Damage by 50% for 3 turns", 0, 0, 96, 0 },
        { "Wrath Potion", 0, 0, 0, 0, 0, 0, 0, 0.15f, 0.75f, 3, "Increases Crit Rate by 15% and Crit Damage by 75% for 3 turns", 0, 0, 108, 0 },
        { "Speed Potion", 0, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 3, "Increases Dodge Rate by 20% for 3 turns", 0, 0, 42, 0 },
        { "Regeneration Potion", 20, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 5, "Restores 20 HP per turn for 5 turns", 0, 0, 84, 0 },
        { "Invisibility Potion", 0, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 2, "Increases Dodge Rate by 50% for 2 turns", 0, 0, 150, 0 }
    };
}
