#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/types.hpp"
#include "catalogs/equipment.hpp"
#include "catalogs/food.hpp"
#include "catalogs/material.hpp"
#include "catalogs/potion.hpp"
#include "catalogs/spell.hpp"
#include "entities/player.hpp"

// Manages the player's items, equipment, and active buffs.
class PlayerInventory {
public:
    enum class ItemType { Potion, Equipment, FoodAndDrink, Material };

    struct InventoryItem {
        ItemType type;
        size_t dbIndex;
        int quantity;
        bool enchanted = false;
        std::string id;
    };

    struct Buff {
        std::string name;
        int attackBonus = 0;
        int defenseBonus = 0;
        int magicAttackBonus = 0;
        int magicDefenseBonus = 0;
        int maxManaBonus = 0;
        float critRateBonus = 0.0f;
        float critDamageBonus = 0.0f;
        int remainingTurns = 0;

        Buff(std::string n, int ab, int db, int mab, int mdb, int mmb, float crb, float cdb, int rt)
            : name(n), attackBonus(ab), defenseBonus(db), magicAttackBonus(mab), magicDefenseBonus(mdb), maxManaBonus(mmb), critRateBonus(crb), critDamageBonus(cdb), remainingTurns(rt) {}
    };

    struct EquippedSlots {
        int armorIndex = -1;
        int weaponIndex = -1;
        int staffIndex = -1;
    };

    void addItem(ItemType type, size_t dbIndex, int amount = 1);
    // Adds only when the backpack has room. Returns false and leaves the
    // inventory untouched when the weight or stack limit would be exceeded.
    bool tryAddItem(ItemType type, size_t dbIndex, int amount = 1);
    void showInventory(Player& player);

    // Stable id of a catalog entry, or an empty string when the index is out of
    // range. Used by saves so content resolves by id, not table position.
    std::string itemId(ItemType type, size_t index) const;
    // Catalog index for an id, or -1 when the id is unknown for that type.
    int indexOfItem(ItemType type, const std::string& id) const;
    // Resolves an id across materials, equipment, potions, and food.
    bool resolveItemId(const std::string& id, ItemType& type, size_t& index) const;
    std::string itemName(ItemType type, size_t index) const;

    int countItem(ItemType type, size_t index) const;
    bool removeItem(ItemType type, size_t index, int amount = 1);

    // Backpack: total weight of everything carried, per-item weight, and the
    // capacity rule. Above capacity you cannot pick up more; above the soft
    // limit combat takes a penalty.
    double weightOf(ItemType type, size_t index) const;
    double totalWeight() const;
    int stackLimitOf(ItemType type, size_t index) const;
    bool canAdd(ItemType type, size_t index, int amount = 1) const;
    bool encumbered() const;

    // Copper the store pays for one unit. Materials use their sell value; other
    // items pay half of their shop price.
    long long sellValueOf(ItemType type, size_t index) const;

    static constexpr double capacityWeight = 50.0;
    static constexpr double softLimit = 0.75;
    void tickBuffs(Player& player);

    const std::optional<Buff>& getActiveBuff() const {
        return activeBuff;
    }

    std::vector<InventoryItem> inventory;
    EquippedSlots equipped;
    std::optional<Buff> activeBuff;

    PotionDatabase potionDB;
    EquipmentandWeaponDatabase equipmentDB;
    FoodandDrinksDatabase foodDB;
    SpellDatabase spellDB;

private:
    std::string getItemName(const InventoryItem& item) const;
    bool isWeapon(const EquipmentandWeaponDatabase::equipmentProperties& eq) const;
    bool isEquipped(size_t index) const;
    void handleSelectedItem(Player& player, InventoryItem& item);
    void usePotion(Player& player, InventoryItem& item);
    void useFoodAndDrink(Player& player, InventoryItem& item);
    void equipItem(Player& player, InventoryItem& item);
    void cleanupInventory();
    void removeBuff(Player& player);
    void equipArmor(Player& player, size_t index);
    void equipWeapon(Player& player, size_t index);
    void unequipArmor(Player& player);
    void unequipWeapon(Player& player);
    void equipStaff(Player& player, size_t index);
    void unequipStaff(Player& player);
    void moveItemToTop(size_t index);
};
