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
        std::string id{};
    };

    SpellDatabase();

    const std::vector<SpellData>& getSpells() const {
        return spellsList;
    }

    // -1 when the id is unknown.
    int indexOf(const std::string& id) const;
    const SpellData* find(const std::string& id) const;

private:
    std::vector<SpellData> spellsList;
};
