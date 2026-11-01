#include "food.hpp"
#include "content.hpp"

FoodandDrinksDatabase::FoodandDrinksDatabase()
    : FoodandDrinkMenu(content::catalog<FoodsandDrinksProperties>("items/food.json")) {}

int FoodandDrinksDatabase::indexOf(const std::string& id) const {
    for (size_t i = 0; i < FoodandDrinkMenu.size(); ++i)
        if (FoodandDrinkMenu[i].id == id) return static_cast<int>(i);
    return -1;
}

const FoodandDrinksDatabase::FoodsandDrinksProperties*
FoodandDrinksDatabase::find(const std::string& id) const {
    int index = indexOf(id);
    return index < 0 ? nullptr : &FoodandDrinkMenu[static_cast<size_t>(index)];
}
