#pragma once

#include <vector>

#include "core/types.hpp"
#include "data/debuff.hpp"
#include "data/food.hpp"
#include "data/potion.hpp"
#include "data/spell.hpp"
#include "entities/npc.hpp"
#include "systems/inventory.hpp"
#include "entities/player.hpp"

struct GameSession;

// Tavern: sleep, buy food/drink, hire party members, and hear actionable rumors.
class Tavern {
public:
    Tavern(PlayerInventory& inv, std::vector<NPC>& party, GameSession& game) : inventory(inv), playerParty(party), session(game) {}

    void openTavern(Player& player, TimeSystem& timeSystem);

private:
    PlayerInventory& inventory;
    std::vector<NPC>& playerParty;
    GameSession& session;
    FoodandDrinksDatabase foodDB;
    NPCGenerator npcGen;

    void buyFoodAndDrinks(Player& player);
    void hirePartyMember(Player& player);
};

// General store: buy potions and equipment.
class Store {
public:
    Store(PlayerInventory& inv) : inventory(inv) {}

    void openStore(Player& player);

private:
    PlayerInventory& inventory;
    PotionDatabase potionDB;
    EquipmentandWeaponDatabase equipmentDB;

    void buyPotions(Player& player);
    void buyEquipment(Player& player);
};

// Magic store: buy spells and enchant items.
class magicStore {
public:
    magicStore(PlayerInventory& inv) : inventory(inv), debuffDB() {}

    void openStore(Player& player);

private:
    PlayerInventory& inventory;
    SpellDatabase spellDB;
    DebuffDatabase debuffDB;

    void buySpells(Player& player);
    void enchantItem(Player& player);
};
