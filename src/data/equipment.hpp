#pragma once

#include <string>
#include <vector>

// Static catalog of weapons, armor, and staves.
class EquipmentandWeaponDatabase {
public:
    struct equipmentProperties {
        std::string name;
        std::string type;
        int defenseIncrease;
        int magicDefenseIncrease;
        int attackIncrease;
        int magicAttackIncrease;
        float critRateIncrease;
        float critDamageIncrease;
        std::vector<std::string> debuffs;
        float debuffChance;
        std::string effectDesc;
        int pricePlatinum;
        int priceGold;
        int priceSilver;
        int priceCopper;
    };

    EquipmentandWeaponDatabase();

    const std::vector<equipmentProperties>& getEquipment() const {
        return equipmentList;
    }

private:
    std::vector<equipmentProperties> equipmentList;
};
