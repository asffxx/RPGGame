#include "inventory.hpp"

#include <algorithm>
#include <iostream>
#include <limits>

#include "core/ui.hpp"
#include "app/display.hpp"

void PlayerInventory::addItem(ItemType type, size_t dbIndex, int amount) {
    for (auto& item : inventory) {
        if (item.type == type && item.dbIndex == dbIndex) {
            item.quantity += amount;
            return;
        }
    }
    inventory.push_back({ type, dbIndex, amount, false, itemId(type, dbIndex) });
}

bool PlayerInventory::tryAddItem(ItemType type, size_t dbIndex, int amount) {
    if (amount <= 0 || !canAdd(type, dbIndex, amount)) return false;
    addItem(type, dbIndex, amount);
    return true;
}

int PlayerInventory::countItem(ItemType type, size_t index) const {
    int total = 0;
    for (const auto& item : inventory)
        if (item.type == type && item.dbIndex == index) total += item.quantity;
    return total;
}

bool PlayerInventory::removeItem(ItemType type, size_t index, int amount) {
    if (amount <= 0) return false;
    if (type == ItemType::Equipment && isEquipped(index) && countItem(type, index) - amount < 1) return false;
    for (auto& item : inventory) {
        if (item.type != type || item.dbIndex != index) continue;
        if (item.quantity < amount) return false;
        item.quantity -= amount;
        cleanupInventory();
        return true;
    }
    return false;
}

double PlayerInventory::weightOf(ItemType type, size_t index) const {
    switch (type) {
        case ItemType::Potion:
            return index < potionDB.getPotions().size() ? 0.5 : 0.0;
        case ItemType::FoodAndDrink:
            return index < foodDB.getFoodAndDrink().size() ? 0.5 : 0.0;
        case ItemType::Equipment:
            if (index >= equipmentDB.getEquipment().size()) return 0.0;
            switch (equipmentDB.getEquipment()[index].type[0]) {
                case 'A': return 8.0;
                case 'S': return 5.0;
                default: return 4.0;
            }
        case ItemType::Material: {
            const auto& materials = materialDatabase().getMaterials();
            return index < materials.size() ? materials[index].weight : 0.0;
        }
    }
    return 0.0;
}

double PlayerInventory::totalWeight() const {
    double total = 0.0;
    for (const auto& item : inventory) total += weightOf(item.type, item.dbIndex) * item.quantity;
    return total;
}

int PlayerInventory::stackLimitOf(ItemType type, size_t index) const {
    if (type == ItemType::Material) {
        const auto& materials = materialDatabase().getMaterials();
        if (index < materials.size()) return materials[index].maxStack;
        return 0;
    }
    return 1000000;
}

bool PlayerInventory::canAdd(ItemType type, size_t index, int amount) const {
    if (amount <= 0) return false;
    double addition = weightOf(type, index) * amount;
    if (addition <= 0.0) return false;
    if (amount > stackLimitOf(type, index) - countItem(type, index)) return false;
    return totalWeight() + addition <= capacityWeight + 1e-9;
}

bool PlayerInventory::encumbered() const {
    return totalWeight() > capacityWeight * softLimit;
}

std::string PlayerInventory::itemId(ItemType type, size_t index) const {
    switch (type) {
        case ItemType::Potion:
            if (index < potionDB.getPotions().size()) return potionDB.getPotions()[index].id;
            break;
        case ItemType::Equipment:
            if (index < equipmentDB.getEquipment().size()) return equipmentDB.getEquipment()[index].id;
            break;
        case ItemType::FoodAndDrink:
            if (index < foodDB.getFoodAndDrink().size()) return foodDB.getFoodAndDrink()[index].id;
            break;
        case ItemType::Material: {
            const auto& materials = materialDatabase().getMaterials();
            if (index < materials.size()) return materials[index].id;
            break;
        }
    }
    return {};
}

int PlayerInventory::indexOfItem(ItemType type, const std::string& id) const {
    switch (type) {
        case ItemType::Potion: return potionDB.indexOf(id);
        case ItemType::Equipment: return equipmentDB.indexOf(id);
        case ItemType::FoodAndDrink: return foodDB.indexOf(id);
        case ItemType::Material: return materialDatabase().indexOf(id);
    }
    return -1;
}

bool PlayerInventory::resolveItemId(const std::string& id, ItemType& type, size_t& index) const {
    for (ItemType candidate : {ItemType::Material, ItemType::Equipment, ItemType::Potion, ItemType::FoodAndDrink}) {
        int found = indexOfItem(candidate, id);
        if (found >= 0) {
            type = candidate;
            index = static_cast<size_t>(found);
            return true;
        }
    }
    return false;
}

std::string PlayerInventory::itemName(ItemType type, size_t index) const {
    return getItemName({type, index, 1, false, itemId(type, index)});
}

long long PlayerInventory::sellValueOf(ItemType type, size_t index) const {
    auto shopPrice = [](int platinum, int gold, int silver, int copper) -> long long {
        return static_cast<long long>(platinum) * 1000000LL + static_cast<long long>(gold) * 10000LL +
               static_cast<long long>(silver) * 100LL + copper;
    };
    switch (type) {
        case ItemType::Potion:
            if (index < potionDB.getPotions().size()) {
                const auto& p = potionDB.getPotions()[index];
                return shopPrice(p.pricePlatinum, p.priceGold, p.priceSilver, p.priceCopper) / 2;
            }
            break;
        case ItemType::Equipment:
            if (index < equipmentDB.getEquipment().size()) {
                const auto& e = equipmentDB.getEquipment()[index];
                return shopPrice(e.pricePlatinum, e.priceGold, e.priceSilver, e.priceCopper) / 2;
            }
            break;
        case ItemType::FoodAndDrink:
            if (index < foodDB.getFoodAndDrink().size()) {
                const auto& f = foodDB.getFoodAndDrink()[index];
                return shopPrice(f.pricePlatinum, f.priceGold, f.priceSilver, f.priceCopper) / 2;
            }
            break;
        case ItemType::Material: {
            const auto& materials = materialDatabase().getMaterials();
            if (index < materials.size()) return materials[index].sellValue;
            break;
        }
    }
    return 0;
}

void PlayerInventory::showInventory(Player& player) {
    displayBorderedMenu({"Backpack: " + std::to_string(totalWeight()) + "/50 weight. Penalties above 37.5.",
        encumbered() ? "Encumbered: attack -10%, dodge -10 percentage points." : "Burden: normal."}, "");
    if (inventory.empty()) {
        display::inventoryEmpty();
        return;
    }

    std::vector<std::string> lines;
    for (size_t i = 0; i < inventory.size(); ++i) {
        std::string itemLine = std::to_string(i + 1) + ". " + getItemName(inventory[i]);
        if (inventory[i].type == ItemType::Equipment && isEquipped(inventory[i].dbIndex)) {
            itemLine += " (Equipped";
            if (inventory[i].enchanted) {
                itemLine += ", Enchanted";
            }
            itemLine += ")";
        }
        if (inventory[i].enchanted && !isEquipped(inventory[i].dbIndex)) {
            itemLine += " [Enchanted]";
        }
        itemLine += " x" + std::to_string(inventory[i].quantity);
        lines.push_back(itemLine);
    }

    displayBorderedMenu(lines, "Choose item (0 to exit): ");
    int choice;
    std::cin >> choice;
    std::cin.ignore();

    if (choice <= 0 || choice > static_cast<int>(inventory.size())) return;

    handleSelectedItem(player, inventory[static_cast<size_t>(choice - 1)]);
}

void PlayerInventory::tickBuffs(Player& player) {
    if (!activeBuff) return;

    activeBuff->remainingTurns--;
    if (activeBuff->remainingTurns <= 0) {
        removeBuff(player);
        display::buffWoreOff(activeBuff->name);
        activeBuff.reset();
    }
}

std::string PlayerInventory::getItemName(const InventoryItem& item) const {
    if (item.type == ItemType::Potion)
        return potionDB.getPotions()[item.dbIndex].name;
    else if (item.type == ItemType::Equipment)
        return equipmentDB.getEquipment()[item.dbIndex].name;
    else if (item.type == ItemType::FoodAndDrink)
        return foodDB.getFoodAndDrink()[item.dbIndex].name;
    else if (item.type == ItemType::Material) {
        const auto& materials = materialDatabase().getMaterials();
        if (item.dbIndex < materials.size()) return materials[item.dbIndex].name;
    }
    return "Unknown Item";
}

bool PlayerInventory::isWeapon(const EquipmentandWeaponDatabase::equipmentProperties& eq) const {
    return eq.attackIncrease > 0 || eq.magicAttackIncrease > 0;
}

bool PlayerInventory::isEquipped(size_t index) const {
    return equipped.armorIndex == static_cast<int>(index) ||
           equipped.weaponIndex == static_cast<int>(index) ||
           equipped.staffIndex == static_cast<int>(index);
}

void PlayerInventory::handleSelectedItem(Player& player, InventoryItem& item) {
    if (item.type == ItemType::Potion)
        usePotion(player, item);
    else if (item.type == ItemType::FoodAndDrink)
        useFoodAndDrink(player, item);
    else if (item.type == ItemType::Material)
        display::message("Materials are used at crafting stations, not from the backpack.");
    else
        equipItem(player, item);
}

void PlayerInventory::usePotion(Player& player, InventoryItem& item) {
    const auto& potion = potionDB.getPotions()[item.dbIndex];

    if (!display::usePotionPrompt(potion.name)) return;


    if (potion.hpEffect > 0) {
        int heal = std::min(potion.hpEffect, player.stats.maxHitpoints - player.stats.hitpoints);
        player.stats.hitpoints += heal;
        display::potionRecoveredHp(heal);
    }


    if (potion.manaRestore > 0) {
        int restore = std::min(potion.manaRestore, player.stats.maxMana - player.stats.mana);
        player.stats.mana += restore;
        display::potionRecoveredMana(restore);
    }


    if (potion.durationTurns > 0) {
        removeBuff(player);

        activeBuff = Buff{
            potion.name,
            potion.attackEffect,
            potion.defenseEffect,
            potion.magicEffect,
            potion.magicDefenseEffect,
            potion.manaIncreaseMax,
            potion.critRateEffect,
            potion.critDamageEffect,
            potion.durationTurns
        };

        player.stats.attack += potion.attackEffect;
        player.stats.armor += static_cast<float>(potion.defenseEffect);
        player.stats.magicAttack += potion.magicEffect;
        player.stats.magicArmor += static_cast<float>(potion.magicDefenseEffect);
        player.stats.critRate += potion.critRateEffect;
        player.stats.critDamage += potion.critDamageEffect;
        if (potion.manaIncreaseMax > 0) {
            player.stats.maxMana += potion.manaIncreaseMax;
            player.stats.mana += potion.manaIncreaseMax;
            }

            display::potionEffect(potion.effectDesc);
        }

    item.quantity--;
    cleanupInventory();
}

void PlayerInventory::useFoodAndDrink(Player& player, InventoryItem& item) {
    const auto& food = foodDB.getFoodAndDrink()[item.dbIndex];

    if (!display::useFoodPrompt(food.name)) return;


    if (food.healthRestoration > 0) {
        int heal = std::min(food.healthRestoration, player.stats.maxHitpoints - player.stats.hitpoints);
        player.stats.hitpoints += heal;
        display::foodRecoveredHp(heal);
    }


    if (food.manaRestoration > 0) {
        int restore = std::min(food.manaRestoration, player.stats.maxMana - player.stats.mana);
        player.stats.mana += restore;
        display::foodRecoveredMana(restore);
    }


    if (food.maxHealthBuffBonus > 0) {
        player.stats.maxHitpoints += food.maxHealthBuffBonus;
        display::foodMaxHpIncrease(food.maxHealthBuffBonus);
    }

    if (food.maxManaBuffBonus > 0) {
        player.stats.maxMana += food.maxManaBuffBonus;
        display::foodMaxManaIncrease(food.maxManaBuffBonus);
    }

    item.quantity--;
    cleanupInventory();
}

void PlayerInventory::equipItem(Player& player, InventoryItem& item) {
    const auto& eq = equipmentDB.getEquipment()[item.dbIndex];
    bool weapon = isWeapon(eq);
    bool staff = eq.type == "Staff";
    bool alreadyEquipped = isEquipped(item.dbIndex);

    if (alreadyEquipped) {
        if (!display::unequipPrompt(eq.name)) return;

        if (staff) unequipStaff(player);
        else if (weapon) unequipWeapon(player);
        else unequipArmor(player);
        display::itemUnequipped(eq.name);
        return;
    }


    if (!display::equipPrompt(eq.name)) return;

    if (staff) equipStaff(player, item.dbIndex);
    else if (weapon) equipWeapon(player, item.dbIndex);
    else equipArmor(player, item.dbIndex);


    auto it = std::find_if(inventory.begin(), inventory.end(), [&](const InventoryItem& i){ return &i == &item; });
    if (it != inventory.end()) moveItemToTop(std::distance(inventory.begin(), it));

    display::itemEffect(eq.effectDesc);
}

void PlayerInventory::cleanupInventory() {
    inventory.erase(
        std::remove_if(inventory.begin(), inventory.end(), [](const InventoryItem& i){ return i.quantity <= 0; }),
        inventory.end()
    );
}

void PlayerInventory::removeBuff(Player& player) {
    if (!activeBuff) return;
    player.stats.attack -= activeBuff->attackBonus;
    player.stats.armor -= static_cast<float>(activeBuff->defenseBonus);
    player.stats.magicAttack -= activeBuff->magicAttackBonus;
    player.stats.magicArmor -= static_cast<float>(activeBuff->magicDefenseBonus);
    player.stats.maxMana -= activeBuff->maxManaBonus;
    player.stats.critRate -= activeBuff->critRateBonus;
    player.stats.critDamage -= activeBuff->critDamageBonus;
    if (player.stats.mana > player.stats.maxMana) {
        player.stats.mana = player.stats.maxMana;
    }
}

void PlayerInventory::equipArmor(Player& player, size_t index) {
    const auto& eq = equipmentDB.getEquipment()[index];
    player.stats.armor += static_cast<float>(eq.defenseIncrease);
    player.stats.magicArmor += static_cast<float>(eq.magicDefenseIncrease);
    equipped.armorIndex = static_cast<int>(index);
}

void PlayerInventory::equipWeapon(Player& player, size_t index) {
    const auto& eq = equipmentDB.getEquipment()[index];
    player.stats.attack += eq.attackIncrease;
    player.stats.magicAttack += eq.magicAttackIncrease;
    player.equippedWeaponDebuffs = eq.debuffs;
    player.equippedWeaponDebuffChance = eq.debuffChance;
    equipped.weaponIndex = static_cast<int>(index);
}

void PlayerInventory::unequipArmor(Player& player) {
    if (equipped.armorIndex == -1) return;
    const auto& eq = equipmentDB.getEquipment()[equipped.armorIndex];
    player.stats.armor -= static_cast<float>(eq.defenseIncrease);
    player.stats.magicArmor -= static_cast<float>(eq.magicDefenseIncrease);
    equipped.armorIndex = -1;
}

void PlayerInventory::unequipWeapon(Player& player) {
    if (equipped.weaponIndex == -1) return;
    const auto& eq = equipmentDB.getEquipment()[equipped.weaponIndex];
    player.stats.attack -= eq.attackIncrease;
    player.stats.magicAttack -= eq.magicAttackIncrease;
    player.equippedWeaponDebuffs.clear();
    player.equippedWeaponDebuffChance = 0.0f;
    equipped.weaponIndex = -1;
}

void PlayerInventory::equipStaff(Player& player, size_t index) {
    const auto& eq = equipmentDB.getEquipment()[index];
    player.stats.magicAttack += eq.magicAttackIncrease;
    equipped.staffIndex = static_cast<int>(index);


    for (const auto& spellName : player.equippedStaffSpells) {
        const auto& spells = spellDB.getSpells();
        auto it = std::find_if(spells.begin(), spells.end(), [&](const SpellDatabase::SpellData& s){ return s.spellName == spellName; });
        if (it != spells.end()) {
            const auto& spell = *it;
            player.stats.magicAttack += static_cast<int>(std::round(spell.healthDamage * spell.staffBonusModifier));
        }
    }
}

void PlayerInventory::unequipStaff(Player& player) {
    if (equipped.staffIndex == -1) return;
    const auto& eq = equipmentDB.getEquipment()[equipped.staffIndex];
    player.stats.magicAttack -= eq.magicAttackIncrease;


    for (const auto& spellName : player.equippedStaffSpells) {
        const auto& spells = spellDB.getSpells();
        auto it = std::find_if(spells.begin(), spells.end(), [&](const SpellDatabase::SpellData& s){ return s.spellName == spellName; });
        if (it != spells.end()) {
            const auto& spell = *it;
            player.stats.magicAttack -= static_cast<int>(std::round(spell.healthDamage * spell.staffBonusModifier));
        }
    }

    equipped.staffIndex = -1;
}

void PlayerInventory::moveItemToTop(size_t index) {
    if (index == 0 || index >= inventory.size()) return;
    InventoryItem item = inventory[index];
    inventory.erase(inventory.begin() + static_cast<int>(index));
    inventory.insert(inventory.begin(), item);
}
