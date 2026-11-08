#include "establishments.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>

#include "core/ui.hpp"
#include "app/display.hpp"
#include "catalogs/guild.hpp"
#include "gameplay/rumor.hpp"
#include "world/reputation.hpp"
#include "world/upgrades.hpp"

namespace {
struct ShopContext {
    ReputationTier tier = ReputationTier::Neutral;
    std::string factionId;
};

ShopContext shopContext(const GameSession& session) {
    const auto* guild = guildDatabase().inCity(session.hero.currentLocation);
    if (!guild) return {};
    return {cityTier(session, guild->city), guild->id};
}

bool inStock(const ShopContext& context, const std::string& itemId) {
    return context.factionId.empty() || stockVisible(context.factionId, context.tier, itemId);
}
}

void Tavern::openTavern(Player& player, TimeSystem& timeSystem) {
    bool inTavern = true;
    while (inTavern) {
        clearScreen();
        std::vector<std::string> lines = {
            "[ The Rattling Flagon Tavern ]",
            "----------------------------",
            "1. Sleep for 1 Gold (Restore HP & Advance Time)",
            "2. Buy Food and Drinks",
            "3. Hire a Party Member (Max 4)",
            "4. Rumors",
            "5. Exit"
        };

        displayBorderedMenu(lines, "The Tavernkeeper wipes a mug. 'What'll it be?' ");

        int choice = getNumberInput(1, 5);

        if (choice == 1) {
            if (player.economy.subtractCurrency(0, 1, 0, 0)) {
                player.stats.hitpoints = player.stats.maxHitpoints;
                player.sleptToday = true;
                timeSystem.advanceTime(player);
                display::message("You settle into a hay-stuffed mattress. The world fades to black... You wake up feeling refreshed! (HP Restored)");
            } else {
                display::message("'No coin, no bed,' the Tavernkeeper grunts.");
            }
        }
        else if (choice == 2) {
            buyFoodAndDrinks(player);
        }
        else if (choice == 3) {
            hirePartyMember(player);
        }
        else if (choice == 4) {
            listenToRumors(session);
        }
        else if (choice == 5) {
            display::message("You step back out into the cold air.");
            inTavern = false;
        }
    }
}

void Tavern::buyFoodAndDrinks(Player& player) {
    const auto& foods = foodDB.getFoodAndDrink();
    const auto context = shopContext(session);
    std::vector<size_t> available;
    std::vector<std::string> foodNames;
    for (size_t i = 0; i < foods.size(); ++i) {
        if (!inStock(context, foods[i].id)) continue;
        available.push_back(i);
        const auto price = adjustedPrice(foods[i].pricePlatinum, foods[i].priceGold,
                                         foods[i].priceSilver, foods[i].priceCopper, context.tier);
        foodNames.push_back(foods[i].name + " (" + priceLabel(price) + ")");
    }

    if (available.empty()) {
        display::message("Nothing in stock at this standing.");
        return;
    }

    PagedSelector foodSelector(foodNames, 5, "Food and Drinks");
    int selected = foodSelector.select();
    if (selected == -1) return;
    const size_t index = available[static_cast<size_t>(selected)];
    const auto& food = foods[index];
    const auto price = adjustedPrice(food.pricePlatinum, food.priceGold,
                                     food.priceSilver, food.priceCopper, context.tier);

    if (!inventory.canAdd(PlayerInventory::ItemType::FoodAndDrink, index)) {
        display::message("Backpack full. Purchase cancelled.");
        return;
    }
    if (player.economy.subtractCurrency(price.platinum, price.gold, price.silver, price.copper)) {
        inventory.addItem(PlayerInventory::ItemType::FoodAndDrink, index);
        display::tavernFoodPurchased(food.name);
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

    displayBorderedMenu({"[ Hire " + newNPC.name + " ]",
        "In the corner, you see " + newNPC.name + ", a " + newNPC.race.name + " " + newNPC.playerClass.name + " (Lv " + std::to_string(newNPC.level) + ").",
        "Hiring cost: " + std::to_string(totalCost) + " Gold.",
        "Gear: " + (newNPC.equippedWeapon.empty() ? std::string("Rags") : newNPC.equippedWeapon),
        "----------------------------",
        "1. Hire member",
        "2. Decline"},
        "Choose: ");
    int confirm = getNumberInput(1, 2);

    if (confirm != 1) {
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


    if (npcGen.isSpecialCharacter(newNPC.name)) {
        player.hiredSpecialCharacters.insert(newNPC.name);
        player.hasNewDictionaryEntry = true;
    }
}

void Store::openStore(Player& player) {
    bool shopping = true;
    while (shopping) {
        clearScreen();
        std::vector<std::string> lines = {
            "[ Store ]",
            "Currency: " + std::to_string(player.economy.platinum) + "p " + std::to_string(player.economy.gold) + "g " +
                std::to_string(player.economy.silver) + "s " + std::to_string(player.economy.copper) + "c",
            "----------------------------",
            "1. Buy Potions",
            "2. Buy Equipment",
            "3. Sell Items",
            "4. Repair Equipment",
            "5. Exit Store",
        };
        displayBorderedMenu(lines, "Choose an option: ");

        int choice = getNumberInput(1, 5);

        switch (choice) {
            case 1:
                buyPotions(player);
                break;
            case 2:
                buyEquipment(player);
                break;
            case 3:
                sellItems(player);
                break;
            case 4:
                openRepairMenu(session);
                break;
            case 5:
                shopping = false;
                break;
        }
    }
}

void Store::buyPotions(Player& player) {
    const auto& potions = potionDB.getPotions();
    const auto context = shopContext(session);
    std::vector<size_t> available;
    std::vector<std::string> potionNames;
    for (size_t i = 0; i < potions.size(); ++i) {
        if (!inStock(context, potions[i].id)) continue;
        available.push_back(i);
        const auto price = adjustedPrice(potions[i].pricePlatinum, potions[i].priceGold,
                                         potions[i].priceSilver, potions[i].priceCopper, context.tier);
        potionNames.push_back(potions[i].name + " - " + priceLabel(price));
    }
    if (available.empty()) {
        display::message("Nothing in stock at this standing.");
        return;
    }
    PagedSelector potionSelector(potionNames, 5, "Buy Potions");
    int selected = potionSelector.select();
    if (selected < 0) return;
    const size_t index = available[static_cast<size_t>(selected)];
    const auto& potion = potions[index];
    const auto price = adjustedPrice(potion.pricePlatinum, potion.priceGold,
                                     potion.priceSilver, potion.priceCopper, context.tier);
    if (!inventory.canAdd(PlayerInventory::ItemType::Potion, index)) {
        display::message("Backpack full. Purchase cancelled.");
        return;
    }
    if (player.economy.subtractCurrency(price.platinum, price.gold, price.silver, price.copper)) {
        inventory.addItem(PlayerInventory::ItemType::Potion, index);
        display::storeBoughtPotion(potion.name);
    } else {
        display::storeNotEnoughCurrency();
    }
}

void Store::buyEquipment(Player& player) {
    const auto& equipment = equipmentDB.getEquipment();
    const auto context = shopContext(session);
    std::vector<size_t> available;
    std::vector<std::string> equipmentNames;
    for (size_t i = 0; i < equipment.size(); ++i) {
        if (!inStock(context, equipment[i].id)) continue;
        available.push_back(i);
        const auto price = adjustedPrice(equipment[i].pricePlatinum, equipment[i].priceGold,
                                         equipment[i].priceSilver, equipment[i].priceCopper, context.tier);
        equipmentNames.push_back(equipment[i].name + " - " + priceLabel(price));
    }
    if (available.empty()) {
        display::message("Nothing in stock at this standing.");
        return;
    }
    PagedSelector equipmentSelector(equipmentNames, 5, "Buy Equipment");
    int selected = equipmentSelector.select();
    if (selected < 0) return;
    const size_t index = available[static_cast<size_t>(selected)];
    const auto& item = equipment[index];
    const auto price = adjustedPrice(item.pricePlatinum, item.priceGold,
                                     item.priceSilver, item.priceCopper, context.tier);
    if (!inventory.canAdd(PlayerInventory::ItemType::Equipment, index)) {
        display::message("Backpack full. Purchase cancelled.");
        return;
    }
    if (player.economy.subtractCurrency(price.platinum, price.gold, price.silver, price.copper)) {
        inventory.addItem(PlayerInventory::ItemType::Equipment, index);
        display::storeBoughtEquipment(item.name);
        if (item.type == "Weapon") {
            player.boughtWeapons.insert(item.name);
            player.hasNewDictionaryEntry = true;
        }
    } else {
        display::storeNotEnoughCurrency();
    }
}

bool sellItem(Player& player, PlayerInventory& inventory, size_t slot, int quantity, std::string& error) {
    error.clear();
    if (slot >= inventory.inventory.size() || quantity <= 0) {
        error = "Invalid sale quantity.";
        return false;
    }
    const auto item = inventory.inventory[slot];
    long long value = inventory.sellValueOf(item.type, item.dbIndex) * quantity;
    if (value <= 0 || quantity > item.quantity || value / 1000000 > 1000000) {
        error = "Cannot sell that quantity.";
        return false;
    }
    if (!inventory.removeItem(item.type, item.dbIndex, quantity)) {
        error = "Unequip the last copy before selling it.";
        return false;
    }
    player.economy.addCurrency(static_cast<int>(value / 1000000), static_cast<int>(value / 10000 % 100),
                              static_cast<int>(value / 100 % 100), static_cast<int>(value % 100));
    return true;
}

void Store::sellItems(Player& player) {
    while (!inventory.inventory.empty()) {
        clearScreen();
        std::vector<std::string> lines;
        for (size_t i = 0; i < inventory.inventory.size(); ++i) {
            const auto& item = inventory.inventory[i];
            lines.push_back(inventory.itemName(item.type, item.dbIndex) +
                " x" + std::to_string(item.quantity) + " - " + std::to_string(inventory.sellValueOf(item.type, item.dbIndex)) + " copper each");
        }
        PagedSelector selector(lines, 5, "Sell Items (shop goods: half price; materials: listed value)");
        int choice = selector.select();
        if (choice < 0) return;
        const auto item = inventory.inventory[static_cast<size_t>(choice)];
        displayBorderedMenu({"[ " + inventory.itemName(item.type, item.dbIndex) + " ]",
            "You hold x" + std::to_string(item.quantity) + ".", "0. Cancel"},
            "Quantity to sell: ");
        int quantity = getNumberInput(0, item.quantity);
        if (!quantity) continue;
        std::string error;
        if (sellItem(player, inventory, static_cast<size_t>(choice), quantity, error))
            display::storeSold(inventory.itemName(item.type, item.dbIndex), quantity, inventory.sellValueOf(item.type, item.dbIndex) * quantity);
        else display::message(error);
    }
    display::storeNothingToSell();
}

void magicStore::openStore(Player& player) {
    bool shopping = true;
    while (shopping) {
        clearScreen();
        std::vector<std::string> lines = {
            "[ Magic Store ]",
            "Currency: " + std::to_string(player.economy.platinum) + "p " + std::to_string(player.economy.gold) + "g " +
                std::to_string(player.economy.silver) + "s " + std::to_string(player.economy.copper) + "c",
            "----------------------------",
            "1. Buy Spells",
            "2. Enchant Equipment",
            "3. Exit Store",
        };
        displayBorderedMenu(lines, "Choose an option: ");

        int choice = getNumberInput(1, 3);

        switch (choice) {
            case 1:
                buySpells(player);
                break;
            case 2:
                enchantItem(player);
                break;
            case 3:
                shopping = false;
                break;
        }
    }
}

void magicStore::buySpells(Player& player) {
    const auto& spells = spellDB.getSpells();
    const auto context = shopContext(session);
    std::vector<size_t> availableSpells;
    for (size_t i = 0; i < spells.size(); ++i) {
        if (spells[i].requiredLevel <= player.stats.level && inStock(context, spells[i].id)) {
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
        const auto price = adjustedPrice(spell.pricePlatinum, spell.priceGold,
                                         spell.priceSilver, spell.priceCopper, context.tier);
        spellNames.push_back(spell.spellName + " - " + priceLabel(price));
    }
    PagedSelector spellSelector(spellNames, 5, "Buy Spells");
    int selectedIndex = spellSelector.select();
    if (selectedIndex < 0) return;
    size_t index = availableSpells[selectedIndex];
    const auto& spell = spells[index];
    const auto price = adjustedPrice(spell.pricePlatinum, spell.priceGold,
                                     spell.priceSilver, spell.priceCopper, context.tier);
    if (player.economy.subtractCurrency(price.platinum, price.gold, price.silver, price.copper)) {

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

    if (item.type == "Weapon") {

        const auto& availableDebuffs = debuffDB.getDebuffs();
        std::vector<std::string> debuffNames;
        for (const auto& debuff : availableDebuffs) {
            debuffNames.push_back(debuff.name + " - " + debuff.effectDesc);
        }
        PagedSelector debuffSelector(debuffNames, 5, "Choose an Enchantment");
        int debuffIndex = debuffSelector.select();
        if (debuffIndex < 0) return;
        if (!player.economy.subtractCurrency(0, 100, 0, 0)) {
            display::magicStoreNotEnoughGold();
            return;
        }
        const auto& debuff = availableDebuffs[debuffIndex];

        player.equippedWeaponDebuffs.push_back(debuff.name);

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
        PagedSelector spellSelector(spellNames, 5, "Choose a Spell");
        int spellIndex = spellSelector.select();
        if (spellIndex < 0) return;
        if (!player.economy.subtractCurrency(0, 100, 0, 0)) {
            display::magicStoreNotEnoughGold();
            return;
        }
        const auto& spell = spells[spellIndex];

        player.equippedStaffSpells.push_back(spell.spellName);

        player.stats.magicAttack += static_cast<int>(std::round(spell.healthDamage * spell.staffBonusModifier));

        display::message("Enchanted " + item.name + " with " + spell.spellName + "!");
    }
}
