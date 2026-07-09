#include "inventory.hpp"

#include <algorithm>
#include <iostream>
#include <limits>

#include "core/ui.hpp"

void PlayerInventory::addItem(ItemType type, size_t dbIndex, int amount) {
    for (auto& item : inventory) {
        if (item.type == type && item.dbIndex == dbIndex) {
            item.quantity += amount;
            return;
        }
    }
    inventory.push_back({ type, dbIndex, amount });
}

void PlayerInventory::showInventory(Player& player) {
    if (inventory.empty()) {
        std::cout << "Inventory is empty.\n";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
        std::cout << activeBuff->name << " has worn off.\n";
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
    else
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
    else
        equipItem(player, item);
}

void PlayerInventory::usePotion(Player& player, InventoryItem& item) {
    const auto& potion = potionDB.getPotions()[item.dbIndex];

    std::cout << "Use " << potion.name << "? (y/n): ";
    char c; std::cin >> c; std::cin.ignore();
    if (c != 'y') return;


    if (potion.hpEffect > 0) {
        int heal = std::min(potion.hpEffect, player.stats.maxHitpoints - player.stats.hitpoints);
        player.stats.hitpoints += heal;
        std::cout << "Recovered " << heal << " HP.\n";
    }


    if (potion.manaRestore > 0) {
        int restore = std::min(potion.manaRestore, player.stats.maxMana - player.stats.mana);
        player.stats.mana += restore;
        std::cout << "Recovered " << restore << " Mana.\n";
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

        std::cout << potion.effectDesc << std::endl;
    }

    item.quantity--;
    cleanupInventory();
}

void PlayerInventory::useFoodAndDrink(Player& player, InventoryItem& item) {
    const auto& food = foodDB.getFoodAndDrink()[item.dbIndex];

    std::cout << "Use " << food.name << "? (y/n): ";
    char c; std::cin >> c; std::cin.ignore();
    if (c != 'y') return;


    if (food.healthRestoration > 0) {
        int heal = std::min(food.healthRestoration, player.stats.maxHitpoints - player.stats.hitpoints);
        player.stats.hitpoints += heal;
        std::cout << "Recovered " << heal << " HP.\n";
    }


    if (food.manaRestoration > 0) {
        int restore = std::min(food.manaRestoration, player.stats.maxMana - player.stats.mana);
        player.stats.mana += restore;
        std::cout << "Recovered " << restore << " Mana.\n";
    }


    if (food.maxHealthBuffBonus > 0) {
        player.stats.maxHitpoints += food.maxHealthBuffBonus;
        std::cout << "Max HP increased by " << food.maxHealthBuffBonus << ".\n";
    }

    if (food.maxManaBuffBonus > 0) {
        player.stats.maxMana += food.maxManaBuffBonus;
        std::cout << "Max Mana increased by " << food.maxManaBuffBonus << ".\n";
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
        std::cout << "Unequip " << eq.name << "? (y/n): ";
        char c; std::cin >> c; std::cin.ignore();
        if (c != 'y') return;

        if (staff) unequipStaff(player);
        else if (weapon) unequipWeapon(player);
        else unequipArmor(player);
        std::cout << eq.name << " unequipped.\n";
        return;
    }


    std::cout << "Equip " << eq.name << "? (y/n): ";
    char c; std::cin >> c; std::cin.ignore();
    if (c != 'y') return;

    if (staff) equipStaff(player, item.dbIndex);
    else if (weapon) equipWeapon(player, item.dbIndex);
    else equipArmor(player, item.dbIndex);


    auto it = std::find_if(inventory.begin(), inventory.end(), [&](const InventoryItem& i){ return &i == &item; });
    if (it != inventory.end()) moveItemToTop(std::distance(inventory.begin(), it));

    std::cout << eq.effectDesc << std::endl;
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
