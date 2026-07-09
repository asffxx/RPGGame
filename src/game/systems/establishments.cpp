#include "establishments.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>

#include "core/ui.hpp"

void Tavern::openTavern(Player& player, TimeSystem& timeSystem) {
    bool inTavern = true;
    while (inTavern) {
        std::vector<std::string> lines = {
            "1. Sleep for 1 Gold (Restore HP & Advance Time)",
            "2. Buy Food and Drinks",
            "3. Hire a Party Member (Max 4)",
            "4. Exit"
        };

        std::cout << "\n--- THE RATTLING FLAGON TAVERN ---" << std::endl;
        displayBorderedMenu(lines, "The Tavernkeeper wipes a mug. 'What'll it be?' ");

        int choice = getNumberInput(1, 4);

        if (choice == 1) {
            if (player.economy.subtractCurrency(0, 1, 0, 0)) {
                player.stats.hitpoints = player.stats.maxHitpoints;
                player.sleptToday = true;
                timeSystem.advanceTime(player);
                std::cout << "\nYou settle into a hay-stuffed mattress. The world fades to black...\n";
                std::cout << "You wake up feeling refreshed! (HP Restored)\n";
            } else {
                std::cout << "\n'No coin, no bed,' the Tavernkeeper grunts.\n";
            }
        }
        else if (choice == 2) {
            buyFoodAndDrinks(player);
        }
        else if (choice == 3) {
            hirePartyMember(player);
        }
        else if (choice == 4) {
            std::cout << "You step back out into the cold air.\n";
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
        std::cout << "You purchased " << foods[index].name << ". It smells... edible.\n";
    } else {
        std::cout << "You don't have enough coin for that.\n";
    }
}

void Tavern::hirePartyMember(Player& player) {
    if (playerParty.size() >= 4) {
        std::cout << "\n'Your group is too big already,' the Tavernkeeper remarks. (Max 4 members)\n";
        return;
    }

    NPC newNPC = npcGen.generateNPC(player.stats.level);


    int baseCost = 10;
    int levelMultiplier = newNPC.level * 2;
    int statBonus = (newNPC.stats.attack + newNPC.stats.magicAttack + newNPC.stats.armor + newNPC.stats.magicArmor) / 10;
    int totalCost = baseCost + levelMultiplier + statBonus;

    std::cout << "\nIn the corner, you see " << newNPC.name << ", a " << newNPC.race.name << " " << newNPC.playerClass.name << " (Lv " << newNPC.level << ").\n";
    std::cout << "Hiring cost: " << totalCost << " Gold.\n";
    std::cout << "Gear: " << (newNPC.equippedWeapon.empty() ? "Rags" : newNPC.equippedWeapon) << "\n";

    std::cout << "Confirm hire? (y/n): ";
    char confirm;
    std::cin >> confirm;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (confirm != 'y' && confirm != 'Y') {
        std::cout << "You decide not to hire them.\n";
        return;
    }

    if (!player.economy.subtractCurrency(0, totalCost, 0, 0)) {
        std::cout << "You realize you can't afford their services.\n";
        return;
    }

    newNPC.wagePerWeek = totalCost / 10;
    playerParty.push_back(newNPC);

    std::cout << "\n" << newNPC.name << " stands up and joins your cause!\n";

    if (!newNPC.dialogues.empty()) {
        static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, newNPC.dialogues.size() - 1);
        std::cout << newNPC.name << " says: \"" << newNPC.dialogues[dist(gen)] << "\"\n" << std::endl;
    }


    std::vector<std::string> specialNames = {"Evelyn Chevalier", "Astra Yao", "Ye Shunguang ", "Burnice White","Jane Doe", "Belle"};
    if (std::find(specialNames.begin(), specialNames.end(), newNPC.name) != specialNames.end()) {
        player.hiredSpecialCharacters.insert(newNPC.name);
        player.hasNewDictionaryEntry = true;
    }
}

void Store::openStore(Player& player) {
    bool shopping = true;
    while (shopping) {
        std::cout << "\n=== STORE ===\n";
        std::cout << "Currency: " << player.economy.platinum << "p " << player.economy.gold << "g " << player.economy.silver << "s " << player.economy.copper << "c" << std::endl;
        std::cout << "1. Buy Potions\n";
        std::cout << "2. Buy Equipment\n";
        std::cout << "3. Exit Store\n";
        std::cout << "Choose an option: ";

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
        std::cout << "Bought " << potions[index].name << "!\n";
    } else {
        std::cout << "Not enough currency!\n";
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
        std::cout << "Bought " << equipment[index].name << "!\n";
        if (equipment[index].type == "Weapon") {
            player.boughtWeapons.insert(equipment[index].name);
            player.hasNewDictionaryEntry = true;
        }
    } else {
        std::cout << "Not enough currency!\n";
    }
}

void magicStore::openStore(Player& player) {
    bool shopping = true;
    while (shopping) {
        std::cout << "\n=== MAGIC STORE ===\n";
        std::cout << "Currency: " << player.economy.platinum << "p " << player.economy.gold << "g " << player.economy.silver << "s " << player.economy.copper << "c" << std::endl;
        std::cout << "1. Buy Spells\n";
        std::cout << "2. Enchant Item\n";
        std::cout << "3. Exit Store\n";
        std::cout << "Choose an option: ";

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
        std::cout << "No spells available for your level.\n";
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
            std::cout << "Learned " << spell.spellName << "!\n";
        } else {
            std::cout << "You already know " << spell.spellName << "!\n";
        }
    } else {
        std::cout << "Not enough currency!\n";
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
        std::cout << "No equipped enchantable items.\n";
        return;
    }

    std::cout << "\n=== ENCHANTABLE ITEMS ===\n";
    for (size_t j = 0; j < enchantableIndices.size(); ++j) {
        size_t i = enchantableIndices[j];
        std::cout << j + 1 << ". " << equipment[i].name << " (" << equipment[i].type << ")\n";
    }
    std::cout << "Choose item to enchant (0 to cancel): ";
    int itemChoice = getNumberInput(0, static_cast<int>(enchantableIndices.size()));
    if (itemChoice == 0) return;

    size_t itemIndex = enchantableIndices[static_cast<size_t>(itemChoice - 1)];
    const auto& item = equipment[itemIndex];

    if (!player.economy.subtractCurrency(0, 100, 0, 0)) {
        std::cout << "Not enough gold! Enchanting costs 100 gold.\n";
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

        std::cout << "Enchanted " << item.name << " with " << debuff.name << "!\n";
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
