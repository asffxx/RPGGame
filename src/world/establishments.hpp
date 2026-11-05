#pragma once

#include <vector>

#include "core/types.hpp"
#include "catalogs/debuff.hpp"
#include "catalogs/food.hpp"
#include "catalogs/potion.hpp"
#include "catalogs/spell.hpp"
#include "entities/npc.hpp"
#include "gameplay/inventory.hpp"
#include "entities/player.hpp"

struct GameSession;
// Sells a quantity atomically, retaining the last equipped copy.
bool sellItem(Player& player, PlayerInventory& inventory, size_t slot, int quantity, std::string& error);

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
    Store(PlayerInventory& inv, GameSession& game) : inventory(inv), session(game) {}

    void openStore(Player& player);

private:
    PlayerInventory& inventory;
    GameSession& session;
    PotionDatabase potionDB;
    EquipmentandWeaponDatabase equipmentDB;

    void buyPotions(Player& player);
    void buyEquipment(Player& player);
    void sellItems(Player& player);
};

// Magic store: buy spells and enchant items.
class magicStore {
public:
    magicStore(PlayerInventory& inv, GameSession& game) : inventory(inv), session(game), debuffDB() {}

    void openStore(Player& player);

private:
    PlayerInventory& inventory;
    GameSession& session;
    SpellDatabase spellDB;
    DebuffDatabase debuffDB;

    void buySpells(Player& player);
    void enchantItem(Player& player);
};
