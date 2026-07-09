#include "food.hpp"

FoodandDrinksDatabase::FoodandDrinksDatabase() {
    FoodandDrinkMenu = {
        // Foods
        {"Bread", "Food", 20, 0, 0, 0, "A loaf of bread that restores some health.", 0,0,1,0},
        {"Apple", "Food", 25, 0, 0, 0, "A fresh apple that restores health.", 0,0,1,50},
        {"Cheese", "Food", 30, 0, 0, 0, "A piece of cheese that restores health.", 0,0,2,0},
        // Dishes
        {"Meat Stew", "Dish", 50, 5, 0, 0, "A hearty stew that restores health and boosts max HP.", 0,0,5,0},
        {"Grilled Fish", "Dish", 45, 3, 0, 0, "Grilled fish that restores health.", 0,0,6,0},
        {"Vegetable Soup", "Dish", 40, 2, 0, 0, "Nutritious soup that restores health.", 0,0,4,50},
        {"Roasted Meat", "Dish", 60, 7, 0, 0, "Roasted meat that greatly restores health.", 0,0,8,0},
        {"Salad", "Dish", 35, 1, 0, 0, "A fresh salad that restores health.", 0,0,3,50},
        // Drinks
        {"Water", "Drink", 0, 0, 0, 10, "Clean water that restores mana.", 0,0,0,50},
        {"Milk", "Drink", 0, 0, 0, 15, "Fresh milk that restores mana.", 0,0,1,0},
        {"Wine", "Drink", 0, 0, 0, 20, "A glass of wine that restores mana.", 0,0,3,0},
        {"Beer", "Drink", 0, 0, 0, 18, "A mug of beer that restores mana.", 0,0,2,50},
        {"Juice", "Drink", 0, 0, 0, 12, "Fruit juice that restores mana.", 0,0,1,50}
    };
}
