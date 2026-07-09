#pragma once

#include <string>
#include <vector>

// Static catalog of potions.
class PotionDatabase {
public:
    struct potionProperties {
        std::string name;
        int hpEffect = 0;
        int attackEffect = 0;
        int defenseEffect = 0;
        int magicEffect = 0;
        int manaRestore = 0;
        int manaIncreaseMax = 0;
        int magicDefenseEffect = 0;
        float critRateEffect = 0.0f;
        float critDamageEffect = 0.0f;
        int durationTurns = 0;
        std::string effectDesc;
        int pricePlatinum;
        int priceGold;
        int priceSilver;
        int priceCopper;
    };

    PotionDatabase();

    const std::vector<potionProperties>& getPotions() const {
        return potionList;
    }

private:
    std::vector<potionProperties> potionList;
};
