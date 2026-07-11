#include "establishments.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>

#include "core/ui.hpp"
#include "app/display.hpp"

void Tavern::openTavern(Player& player, TimeSystem& timeSystem) {
    bool inTavern = true;
    while (inTavern) {
        std::vector<std::string> lines = {
            "1. Sleep for 1 Gold (Restore HP & Advance Time)",
            "2. Buy Food and Drinks",
            "3. Hire a Party Member (Max 4)",
            "4. Exit"
        };

        display::tavernHeader();
        displayBorderedMenu(lines, "The Tavernkeeper wipes a mug. 'What'll it be?' ");

        int choice = getNumberInput(1, 4);

        if (choice == 1) {
            if (player.economy.subtractCurrency(0, 1, 0, 0)) {
                player.stats.hitpoints = player.stats.maxHitpoints;
                player.sleptToday = true;
                timeSystem.advanceTime(player);
                display::tavernSlept();
            } else {
                display::tavernNoCoin();
            }
        }
        else if (choice == 2) {
            buyFoodAndDrinks(player);
        }
        else if (choice == 3) {
            hirePartyMember(player);
        }
        else if (choice == 4) {
            display::tavernExit();
            inTavern = false;
        }
    }
}

void Tavern::buyFoodAndDrinks(Player& player) {
    const auto& foods = foodDB.getFoodAndDrink();
    std::vector<std::string> foodNames;
    for (const auto& food : foods) {

        std::string price = std::to_string(food.priceGold) + "g " + std::to_string(food.priceSilver) + "s";
        foodNames.push_back(food.name + " (" + price + ")");
    }

    std::cout << "\n--- Available ---" << std::endl;
    PagedSelector foodSelector(foodNames);
    int index = foodSelector.select();

    if (index == -1) return;

    if (player.economy.subtractCurrency(foods[index].pricePlatinum, foods[index].priceGold, foods[index].priceSilver, foods[index].priceCopper)) {
        inventory.addItem(PlayerInventory::ItemType::FoodAndDrink, index);
        display::tavernFoodPurchased(foods[index].name);
    } else {
        display::tavernFoodTooExpensive();
    }
}

void Tavern::hirePartyMember(Player& player) {
    if (playerParty.size() >= 4) {
        display::tavernPartyTooBig();
        return;
    }

    NPC newNPC = npcGen.generateNPC(player.stats.level);


    int baseCost = 10;
    int levelMultiplier = newNPC.level * 2;
    int statBonus = (newNPC.stats.attack + newNPC.stats.magicAttack + newNPC.stats.armor + newNPC.stats.magicArmor) / 10;
    int totalCost = baseCost + levelMultiplier + statBonus;

    display::tavernHireInfo(newNPC, totalCost);

    std::cout << "Confirm hire? (y/n): ";
    char confirm;
    std::cin >> confirm;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (confirm != 'y' && confirm != 'Y') {
        display::tavernHireDeclined();
        return;
    }

    if (!player.economy.subtractCurrency(0, totalCost, 0, 0)) {
        display::tavernHireTooExpensive();
        return;
    }

    newNPC.wagePerWeek = totalCost / 10;
    playerParty.push_back(newNPC);

    std::string hireLine;
    if (!newNPC.dialogues.empty()) {
        static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, newNPC.dialogues.size() - 1);
        const char quote = '"';
        hireLine = newNPC.name + " says: " + quote + newNPC.dialogues[dist(gen)] + quote;
    }
    display::tavernHireJoined(newNPC, hireLine);


    std::vector<std::string> specialNames = {"Evelyn Chevalier", "Astra Yao", "Ye Shunguang ", "Burnice White","Jane Doe", "Belle"};
    if (std::find(specialNames.begin(), specialNames.end(), newNPC.name) != specialNames.end()) {
        player.hiredSpecialCharacters.insert(newNPC.name);
        player.hasNewDictionaryEntry = true;
    }
}

void Store::openStore(Player& player) {
    bool shopping = true;
    while (shopping) {
        display::storeHeader(player);

        int choice = getNumberInput(1, 3);

        switch (choice) {
            case 1:
                buyPotions(player);
                system("cls");
                break;
            case 2:
                buyEquipment(player);
                system("cls");
                break;
            case 3:
                shopping = false;
                break;
        }
    }
}

void Store::buyPotions(Player& player) {
    const auto& potions = potionDB.getPotions();
    std::vector<std::string> potionNames;
    for (const auto& potion : potions) {
        potionNames.push_back(potion.name + " - " + std::to_string(potion.priceSilver) + "s " + std::to_string(potion.priceCopper) + "c");
    }
    PagedSelector potionSelector(potionNames);
    size_t index = potionSelector.select();
    if (player.economy.subtractCurrency(0, 0, potions[index].priceSilver, potions[index].priceCopper)) {
        inventory.addItem(PlayerInventory::ItemType::Potion, index);
        display::storeBoughtPotion(potions[index].name);
    } else {
        display::storeNotEnoughCurrency();
    }
}

void Store::buyEquipment(Player& player) {
    const auto& equipment = equipmentDB.getEquipment();
    std::vector<std::string> equipmentNames;
    for (const auto& eq : equipment) {
        equipmentNames.push_back(eq.name + " - " + std::to_string(eq.priceSilver) + "s " + std::to_string(eq.priceCopper) + "c");
    }
    PagedSelector equipmentSelector(equipmentNames);
    size_t index = equipmentSelector.select();
    if (player.economy.subtractCurrency(0, 0, equipment[index].priceSilver, equipment[index].priceCopper)) {
        inventory.addItem(PlayerInventory::ItemType::Equipment, index);
        display::storeBoughtEquipment(equipment[index].name);
        if (equipment[index].type == "Weapon") {
            player.boughtWeapons.insert(equipment[index].name);
            player.hasNewDictionaryEntry = true;
        }
    } else {
        display::storeNotEnoughCurrency();
    }
}

void magicStore::openStore(Player& player) {
    bool shopping = true;
    while (shopping) {
        display::magicStoreHeader(player);

        int choice = getNumberInput(1, 3);

        switch (choice) {
            case 1:
                buySpells(player);
                system("cls");
                break;
            case 2:
                enchantItem(player);
                system("cls");
                break;
            case 3:
                shopping = false;
                break;
        }
    }
}

void magicStore::buySpells(Player& player) {
    const auto& spells = spellDB.getSpells();
    std::vector<size_t> availableSpells;
    for (size_t i = 0; i < spells.size(); ++i) {
        if (spells[i].requiredLevel <= player.stats.level) {
            availableSpells.push_back(i);
        }
    }

    if (availableSpells.empty()) {
        display::magicStoreNoSpells();
        return;
    }

    std::vector<std::string> spellNames;
    for (size_t i : availableSpells) {
        const auto& spell = spells[i];
        spellNames.push_back(spell.spellName + " - " + std::to_string(spell.pricePlatinum) + "p " + std::to_string(spell.priceGold) + "g " + std::to_string(spell.priceSilver) + "s " + std::to_string(spell.priceCopper) + "c");
    }
    PagedSelector spellSelector(spellNames);
    size_t selectedIndex = spellSelector.select();
    size_t index = availableSpells[selectedIndex];
    const auto& spell = spells[index];
    if (player.economy.subtractCurrency(spell.pricePlatinum, spell.priceGold, spell.priceSilver, spell.priceCopper)) {

        if (std::find(player.learnedSpells.begin(), player.learnedSpells.end(), spell.spellName) == player.learnedSpells.end()) {
            player.learnedSpells.push_back(spell.spellName);
            display::magicStoreLearned(spell.spellName);
        } else {
            display::magicStoreAlreadyKnown(spell.spellName);
        }
    } else {
        display::storeNotEnoughCurrency();
    }
}

void magicStore::enchantItem(Player& player) {

    std::vector<size_t> enchantableIndices;
    const auto& equipment = inventory.equipmentDB.getEquipment();
    for (size_t i = 0; i < equipment.size(); ++i) {
        if (equipment[i].type == "Weapon" || equipment[i].type == "Staff") {

            bool isEquipped = (equipment[i].type == "Weapon" && inventory.equipped.weaponIndex == static_cast<int>(i)) ||
                              (equipment[i].type == "Staff" && inventory.equipped.staffIndex == static_cast<int>(i));
            if (isEquipped) {
                enchantableIndices.push_back(i);
            }
        }
    }

    if (enchantableIndices.empty()) {
        display::magicStoreNoEnchantable();
        return;
    }

    display::magicStoreEnchantItemHeader();
    std::vector<std::string> enchantLines;
    for (size_t j = 0; j < enchantableIndices.size(); ++j) {
        size_t i = enchantableIndices[j];
        enchantLines.push_back(equipment[i].name + " (" + equipment[i].type + ")");
    }
    display::magicStoreEnchantChoice(enchantLines);
    int itemChoice = getNumberInput(0, static_cast<int>(enchantableIndices.size()));
    if (itemChoice == 0) return;

    size_t itemIndex = enchantableIndices[static_cast<size_t>(itemChoice - 1)];
    const auto& item = equipment[itemIndex];

    if (!player.economy.subtractCurrency(0, 100, 0, 0)) {
        display::magicStoreNotEnoughGold();
        return;
    }

    if (item.type == "Weapon") {

        const auto& availableDebuffs = debuffDB.getDebuffs();
        std::vector<std::string> debuffNames;
        for (const auto& debuff : availableDebuffs) {
            debuffNames.push_back(debuff.name + " - " + debuff.effectDesc);
        }
        PagedSelector debuffSelector(debuffNames);
        size_t debuffIndex = debuffSelector.select();
        const auto& debuff = availableDebuffs[debuffIndex];

        player.equippedWeaponDebuffs.push_back(debuff.name);
        player.equippedWeaponDebuffChance = item.debuffChance;

        for (auto& invItem : inventory.inventory) {
            if (invItem.type == PlayerInventory::ItemType::Equipment && invItem.dbIndex == itemIndex) {
                invItem.enchanted = true;
            }
        }

        display::magicStoreEnchanted(item.name, debuff.name);
    } else if (item.type == "Staff") {

        const auto& spells = spellDB.getSpells();
        std::vector<std::string> spellNames;
        for (const auto& spell : spells) {
            spellNames.push_back(spell.spellName + " - " + spell.description);
        }
        PagedSelector spellSelector(spellNames);
        size_t spellIndex = spellSelector.select();
        const auto& spell = spells[spellIndex];

        player.equippedStaffSpells.push_back(spell.spellName);

        player.stats.magicAttack += static_cast<int>(std::round(spell.healthDamage * spell.staffBonusModifier));

        std::cout << "Enchanted " << item.name << " with " << spell.spellName << "!\n";
    }
}
