#pragma once

#include <string>
#include <vector>

// Static catalog of spells.
class SpellDatabase {
public:
    struct SpellData {
        std::string spellName;

        int manaCost;

        // Damage
        int healthDamage;
        int manaDamage;

        // Restoration
        int healthRestore;
        int manaRestore;

        // Buffs
        float armorIncrease;
        float magicArmorIncrease;

        // Debuffs
        float armorReduction;
        float magicArmorReduction;

        bool hasDebuff;
        std::vector<std::string> debuffs;
        float debuffModifierOnHit;

        // Weapon interaction
        bool applicableOnStaffs;
        float staffBonusModifier;

        int requiredLevel;
        std::string description;
        int pricePlatinum;
        int priceGold;
        int priceSilver;
        int priceCopper;
    };

    SpellDatabase();

    const std::vector<SpellData>& getSpells() const {
        return spellsList;
    }

private:
    std::vector<SpellData> spellsList;
};
