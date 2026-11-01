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
        std::string id{};
    };

    FoodandDrinksDatabase();

    const std::vector<FoodsandDrinksProperties>& getFoodAndDrink() const {
        return FoodandDrinkMenu;
    }

    // -1 when the id is unknown.
    int indexOf(const std::string& id) const;
    const FoodsandDrinksProperties* find(const std::string& id) const;

private:
    std::vector<FoodsandDrinksProperties> FoodandDrinkMenu;
};
