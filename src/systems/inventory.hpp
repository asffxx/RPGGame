#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/types.hpp"
#include "data/equipment.hpp"
#include "data/food.hpp"
#include "data/potion.hpp"
#include "data/spell.hpp"
#include "entities/player.hpp"

// Manages the player's items, equipment, and active buffs.
class PlayerInventory {
public:
    enum class ItemType { Potion, Equipment, FoodAndDrink };

    struct InventoryItem {
        ItemType type;
        size_t dbIndex;
        int quantity;
        bool enchanted = false;
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
    void showInventory(Player& player);
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
