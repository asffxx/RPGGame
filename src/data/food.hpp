#pragma once

#include <string>
#include <vector>

// Static catalog of consumable food and drinks.
class FoodandDrinksDatabase {
public:
    struct FoodsandDrinksProperties {
        std::string name;
        std::string type;
        int healthRestoration;
        int maxHealthBuffBonus;
        int maxManaBuffBonus;
        int manaRestoration;
        std::string itemDesc;

        int pricePlatinum;
        int priceGold;
        int priceSilver;
        int priceCopper;
    };

    FoodandDrinksDatabase();

    const std::vector<FoodsandDrinksProperties>& getFoodAndDrink() const {
        return FoodandDrinkMenu;
    }

private:
    std::vector<FoodsandDrinksProperties> FoodandDrinkMenu;
};
