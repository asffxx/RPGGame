#include "display.hpp"

#include <iostream>
#include <limits>

#include "core/ui.hpp"

namespace display {

void pressEnter() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void invalidChoice() {
    std::cout << "Invalid choice!" << std::endl;
}

void sleepPenalty() {
    std::cout << "[!] Your vision blurs. The shadows of 'The Lands Between' seem longer than usual." << std::endl;
}

void enemyAppeared(const Enemy& enemy) {
    std::cout << "A wild " << enemy.name << " has appeared!" << std::endl;
}

void enemyDefeated(const Enemy& enemy, const Player& player) {
    std::cout << "You have defeated " << enemy.name
         << ", you gained " << enemy.stats.data.economy.platinum << "p " << enemy.stats.data.economy.gold << "g " << enemy.stats.data.economy.silver << "s " << enemy.stats.data.economy.copper << "c"
         << " and " << enemy.stats.data.expe
         << " experience!" << std::endl;
}

void combatRunAway() {
    std::cout << "You ran away!" << std::endl;
}

void combatInvalidChoice() {
    std::cout << "Invalid choice!" << std::endl;
}

void combatSpellPrompt(const std::vector<std::string>& spellNames) {
    std::cout << "Choose a spell to cast:" << std::endl;
    for (size_t i = 0; i < spellNames.size(); ++i) {
        std::cout << i + 1 << ". " << spellNames[i] << std::endl;
    }
}

void combatManaInsufficient() {
    std::cout << "Mana insufficient" << std::endl;
}

void combatVictory(const std::string& enemyName) {
    std::cout << "\n========== VICTORY ==========\n";
    std::cout << enemyName << " has been defeated!\n";
    std::cout << "=============================\n";
}

void combatDefeat(const std::string& playerName) {
    std::cout << "\n========== DEFEAT ==========\n";
    std::cout << playerName << " has been defeated!\n";
    std::cout << "============================\n";
}

void inventoryEmpty() {
    std::cout << "Inventory is empty.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void buffWoreOff(const std::string& name) {
    std::cout << name << " has worn off.\n";
}

bool usePotionPrompt(const std::string& potionName) {
    std::cout << "Use " << potionName << "? (y/n): ";
    char c; std::cin >> c; std::cin.ignore();
    return c == 'y';
}

void potionRecoveredHp(int heal) {
    std::cout << "Recovered " << heal << " HP.\n";
}

void potionRecoveredMana(int restore) {
    std::cout << "Recovered " << restore << " Mana.\n";
}

void potionEffect(const std::string& effectDesc) {
    std::cout << effectDesc << std::endl;
}

bool useFoodPrompt(const std::string& foodName) {
    std::cout << "Use " << foodName << "? (y/n): ";
    char c; std::cin >> c; std::cin.ignore();
    return c == 'y';
}

void foodRecoveredHp(int heal) {
    std::cout << "Recovered " << heal << " HP.\n";
}

void foodRecoveredMana(int restore) {
    std::cout << "Recovered " << restore << " Mana.\n";
}

void foodMaxHpIncrease(int amount) {
    std::cout << "Max HP increased by " << amount << ".\n";
}

void foodMaxManaIncrease(int amount) {
    std::cout << "Max Mana increased by " << amount << ".\n";
}

bool unequipPrompt(const std::string& itemName) {
    std::cout << "Unequip " << itemName << "? (y/n): ";
    char c; std::cin >> c; std::cin.ignore();
    return c == 'y';
}

void itemUnequipped(const std::string& itemName) {
    std::cout << itemName << " unequipped.\n";
}

bool equipPrompt(const std::string& itemName) {
    std::cout << "Equip " << itemName << "? (y/n): ";
    char c; std::cin >> c; std::cin.ignore();
    return c == 'y';
}

void itemEffect(const std::string& effectDesc) {
    std::cout << effectDesc << std::endl;
}

void eventHeader(const std::string& name, const std::string& description) {
    std::cout << "\n=== EVENT: " << name << " ===" << std::endl;
    std::cout << description << std::endl;
}

void eventGoldGained(int amount) {
    std::cout << "You gained " << amount << " silver!" << std::endl;
}

void eventGoldLost(int amount) {
    std::cout << "You lost " << amount << " silver!" << std::endl;
}

void eventNotEnoughGold() {
    std::cout << "You didn't have enough gold to lose!" << std::endl;
}

void eventExpGained(int amount) {
    std::cout << "You gained " << amount << " experience!" << std::endl;
}

void eventExpLost(int amount) {
    std::cout << "You lost " << amount << " experience!" << std::endl;
}

void eventHpRecovered(int heal) {
    std::cout << "You recovered " << heal << " HP!" << std::endl;
}

void eventHpDamage(int damage, bool survived) {
    if (survived) {
        std::cout << "You took " << damage << " damage but survived!" << std::endl;
    } else {
        std::cout << "You took " << damage << " damage!" << std::endl;
    }
}

void eventManaRecovered(int restore) {
    std::cout << "You recovered " << restore << " Mana!" << std::endl;
}

void eventManaLost(int drain) {
    std::cout << "You lost " << drain << " Mana!" << std::endl;
}

void eventEnemyEncounter() {
    std::cout << "\nYou encounter an enemy!" << std::endl;
}

void eventPressEnter() {
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void wagesPaid(int total) {
    std::cout << "Paid " << total << " gold in wages to party members.\n";
}

void wagesInsufficient() {
    std::cout << "Not enough gold to pay wages! Party members may become unhappy.\n";
}

void partyEmpty() {
    std::cout << "Your party is empty.\n";
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void partyMemberLeft(const std::string& name) {
    std::cout << "Due to insufficient gold, " << name << " has left the party.\n";
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void partyMemberList(const std::vector<NPC>& party) {
    std::vector<std::string> lines;
    lines.push_back("Current Party Members:");
    for (size_t i = 0; i < party.size(); ++i) {
        const auto& npc = party[i];
        std::string line = std::to_string(i + 1) + ". " + npc.name + " (" + npc.race.name + " " + npc.playerClass.name + ", Level " + std::to_string(npc.level) + ") - Weekly Wage: " + std::to_string(npc.wagePerWeek) + " gold";
        lines.push_back(line);
    }
    lines.push_back("Select a member (0 to cancel):");
    displayBorderedMenu(lines, "");
}

void partyActionPrompt(const std::string& name) {
    std::cout << "What do you want to do with " << name << "?" << std::endl;
    std::cout << "1. Talk to member" << std::endl;
    std::cout << "2. What is your story?" << std::endl;
    std::cout << "3. Kick member" << std::endl;
    std::cout << "4. Cancel" << std::endl;
}

void partyDialogue(const std::string& name, const std::string& line) {
    std::cout << name << " says: \"" << line << "\"" << std::endl;
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void partyKicked(const std::string& name) {
    std::cout << name << " has been kicked from the party.\n";
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void partyStory(const std::string& name, const std::string& story) {
    std::cout << name << "'s story: " << story << std::endl;
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEmptyEnemies() {
    std::cout << "No defeated enemies yet.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEmptyWeapons() {
    std::cout << "No bought weapons yet.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEmptyLocations() {
    std::cout << "No discovered locations yet.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEmptyEvents() {
    std::cout << "No encountered events yet.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEmptyCharacters() {
    std::cout << "No hired special characters yet.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEnemyHeader(const std::string& name) {
    std::cout << "=== " << name << " ===\n";
    std::cout << "1. Stats\n";
    std::cout << "2. Description\n";
    std::cout << "3. Exit\n";
    std::cout << "Choose: ";
}

void dictionaryEnemyStats(const EnemyDatabase::EnemyTemplate& tmpl) {
    std::cout << "HP: " << tmpl.stats.data.hitpoints << "/" << tmpl.stats.data.maxHitpoints << "\n";
    std::cout << "Attack: " << tmpl.stats.data.attack << "\n";
    std::cout << "Magic Attack: " << tmpl.stats.data.magicAttack << "\n";
    std::cout << "Armor: " << tmpl.stats.data.armor << "\n";
    std::cout << "Magic Armor: " << tmpl.stats.data.magicArmor << "\n";
    std::cout << "Level: " << tmpl.stats.data.level << "\n";
    std::cout << "Experience: " << tmpl.stats.data.expe << "\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEnemyDescription(const std::string& desc) {
    std::cout << desc << "\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryWeaponHeader(const std::string& name) {
    std::cout << "=== " << name << " ===\n";
    std::cout << "1. Stats\n";
    std::cout << "2. Description\n";
    std::cout << "3. Exit\n";
    std::cout << "Choose: ";
}

void dictionaryWeaponStats(const EquipmentandWeaponDatabase::equipmentProperties& eq) {
    std::cout << "Type: " << eq.type << "\n";
    std::cout << "Attack Increase: " << eq.attackIncrease << "\n";
    std::cout << "Magic Attack Increase: " << eq.magicAttackIncrease << "\n";
    std::cout << "Crit Rate Increase: " << eq.critRateIncrease << "\n";
    std::cout << "Crit Damage Increase: " << eq.critDamageIncrease << "\n";
    std::cout << "Price: " << eq.pricePlatinum << "p " << eq.priceGold << "g " << eq.priceSilver << "s " << eq.priceCopper << "c\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryWeaponDescription(const std::string& desc) {
    std::cout << desc << "\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryLocationHeader(const std::string& name) {
    std::cout << "=== " << name << " ===\n";
    std::cout << "1. Stats\n";
    std::cout << "2. Description\n";
    std::cout << "3. Exit\n";
    std::cout << "Choose: ";
}

void dictionaryLocationStats(const locationDatabase::locationProperties& loc) {
    std::cout << "Difficulty Level: " << loc.difficultyLevel << "\n";
    std::string typeStr;
    switch (loc.type) {
        case PeacefulVillage: typeStr = "Peaceful Village"; break;
        case PeacefulTown: typeStr = "Peaceful Town"; break;
        case Dungeon: typeStr = "Dungeon"; break;
        case Terrain: typeStr = "Terrain"; break;
        case SpellStore: typeStr = "Spell Store"; break;
    }
    std::cout << "Type: " << typeStr << "\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryLocationDescription(const std::string& desc) {
    std::cout << desc << "\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEventHeader(const std::string& name) {
    std::cout << "=== " << name << " ===\n";
    std::cout << "1. Stats\n";
    std::cout << "2. Description\n";
    std::cout << "3. Exit\n";
    std::cout << "Choose: ";
}

void dictionaryEventStats(const eventDatabase::eventProperties& ev) {
    std::cout << "Gold Reward: " << ev.goldReward << "\n";
    std::cout << "Exp Reward: " << ev.expReward << "\n";
    std::cout << "Health Effect: " << ev.healthEffect << "\n";
    std::cout << "Mana Effect: " << ev.manaEffect << "\n";
    std::cout << "Enemy Encounter: " << (ev.enemyEncounter ? "Yes" : "No") << "\n";
    if (ev.enemyEncounter) {
        std::cout << "Enemy: " << ev.enemyName << "\n";
        std::cout << "Count: " << ev.enemyCount << "\n";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEventDescription(const std::string& desc) {
    std::cout << desc << "\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionarySpecialCharacters(const std::vector<std::string>& names) {
    std::cout << "=== Special Characters ===\n";
    for (const auto& name : names) {
        std::cout << "- " << name << "\n";
    }
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void menuInvalidChoice() {
    std::cout << "Invalid choice!" << std::endl;
}

void showPlayerStats(const Player& player, float reqAmount, const std::optional<PlayerInventory::Buff>& activeBuff) {
    std::cout << "\n=== Player Stats ===" << std::endl;
    std::cout << "Hitpoints: " << player.stats.hitpoints << "/" << player.stats.maxHitpoints << std::endl;

    if (activeBuff) {
        std::cout << "Armor: " << player.stats.armor
             << "( + " << activeBuff->defenseBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << std::endl;
        std::cout << "Magic Armor: " << player.stats.magicArmor
             << "( + " << activeBuff->magicDefenseBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << std::endl;
        std::cout << "Attack: " << player.stats.attack
             << "( + " << activeBuff->attackBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << std::endl;
        std::cout << "Magic Attack: " << player.stats.magicAttack
             << "( + " << activeBuff->magicAttackBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << std::endl;
    } else {
        std::cout << "Armor: " << player.stats.armor << std::endl;
        std::cout << "Magic Armor: " << player.stats.magicArmor << std::endl;
        std::cout << "Attack: " << player.stats.attack << std::endl;
        std::cout << "Magic Attack: " << player.stats.magicAttack << std::endl;
    }

    std::cout << "Level: " << player.stats.level << std::endl;
    std::cout << "Experience: " << player.stats.expe << "/" << reqAmount << std::endl;
    std::cout << "Currency: " << player.economy.platinum << "p " << player.economy.gold << "g " << player.economy.silver << "s " << player.economy.copper << "c" << std::endl;
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void leveledUp() {
    std::cout << "You leveled Up!" << std::endl;
    std::cout << "Your stats have been increased by 10%." << std::endl;
}

void tavernHeader() {
    std::cout << "\n--- THE RATTLING FLAGON TAVERN ---" << std::endl;
}

void tavernSlept() {
    std::cout << "\nYou settle into a hay-stuffed mattress. The world fades to black...\n";
    std::cout << "You wake up feeling refreshed! (HP Restored)\n";
}

void tavernNoCoin() {
    std::cout << "\n'No coin, no bed,' the Tavernkeeper grunts.\n";
}

void tavernExit() {
    std::cout << "You step back out into the cold air.\n";
}

void tavernFoodPurchased(const std::string& name) {
    std::cout << "You purchased " << name << ". It smells... edible.\n";
}

void tavernFoodTooExpensive() {
    std::cout << "You don't have enough coin for that.\n";
}

void tavernPartyTooBig() {
    std::cout << "\n'Your group is too big already,' the Tavernkeeper remarks. (Max 4 members)\n";
}

void tavernHireInfo(const NPC& npc, int totalCost) {
    std::cout << "\nIn the corner, you see " << npc.name << ", a " << npc.race.name << " " << npc.playerClass.name << " (Lv " << npc.level << ").\n";
    std::cout << "Hiring cost: " << totalCost << " Gold.\n";
    std::cout << "Gear: " << (npc.equippedWeapon.empty() ? "Rags" : npc.equippedWeapon) << "\n";
}

void tavernHireDeclined() {
    std::cout << "You decide not to hire them.\n";
}

void tavernHireTooExpensive() {
    std::cout << "You realize you can't afford their services.\n";
}

void tavernHireJoined(const NPC& npc, const std::string& line) {
    std::cout << "\n" << npc.name << " stands up and joins your cause!\n";
    std::cout << line << "\n" << std::endl;
}

void storeHeader(const Player& player) {
    std::cout << "\n=== STORE ===\n";
    std::cout << "Currency: " << player.economy.platinum << "p " << player.economy.gold << "g " << player.economy.silver << "s " << player.economy.copper << "c" << std::endl;
    std::cout << "1. Buy Potions\n";
    std::cout << "2. Buy Equipment\n";
    std::cout << "3. Exit Store\n";
    std::cout << "Choose an option: ";
}

void storeBoughtPotion(const std::string& name) {
    std::cout << "Bought " << name << "!\n";
}

void storeBoughtEquipment(const std::string& name) {
    std::cout << "Bought " << name << "!\n";
}

void storeNotEnoughCurrency() {
    std::cout << "Not enough currency!\n";
}

void magicStoreHeader(const Player& player) {
    std::cout << "\n=== MAGIC STORE ===\n";
    std::cout << "Currency: " << player.economy.platinum << "p " << player.economy.gold << "g " << player.economy.silver << "s " << player.economy.copper << "c" << std::endl;
    std::cout << "1. Buy Spells\n";
    std::cout << "2. Enchant Item\n";
    std::cout << "3. Exit Store\n";
    std::cout << "Choose an option: ";
}

void magicStoreNoSpells() {
    std::cout << "No spells available for your level.\n";
}

void magicStoreLearned(const std::string& name) {
    std::cout << "Learned " << name << "!\n";
}

void magicStoreAlreadyKnown(const std::string& name) {
    std::cout << "You already know " << name << "!\n";
}

void magicStoreNoEnchantable() {
    std::cout << "No equipped enchantable items.\n";
}

void magicStoreEnchantItemHeader() {
    std::cout << "\n=== ENCHANTABLE ITEMS ===\n";
}

void magicStoreEnchantChoice(const std::vector<std::string>& lines) {
    for (size_t j = 0; j < lines.size(); ++j) {
        std::cout << j + 1 << ". " << lines[j] << "\n";
    }
    std::cout << "Choose item to enchant (0 to cancel): ";
}

void magicStoreNotEnoughGold() {
    std::cout << "Not enough gold! Enchanting costs 100 gold.\n";
}

void magicStoreEnchanted(const std::string& itemName, const std::string& applied) {
    std::cout << "Enchanted " << itemName << " with " << applied << "!\n";
}

void travelNoMarkedLocations() {
    std::cout << "No marked locations.\n";
}

void travelLocationList(const std::vector<std::string>& lines) {
    for (const auto& line : lines) {
        std::cout << line << std::endl;
    }
    std::cout << "Choose a location: ";
}

void travelDiscovered(const std::string& name) {
    std::cout << "\nDiscovered new location: " << name << "! Gained 50 experience.\n";
}

void travelLocationHeader(const std::string& name, const std::string& description) {
    std::cout << "\n=== " << name << " ===\n";
    std::cout << description << std::endl;
    std::cout << "1. Explore\n";
    std::cout << "2. Mark location\n";
    std::cout << "3. Leave\n";
    std::cout << "Choose an action: ";
}

void travelLocationMarked() {
    std::cout << "Location marked for safe travel.\n";
}

void timePassed() {
    std::cout << "Time has passed.\n";
}

void gameBasics() {
    std::cout << "\n=== GAME BASICS ===\n";
    std::cout << "Time System:\n";
    std::cout << "- 4 turns advance the state of the day (Morning -> Afternoon -> Evening -> Night).\n";
    std::cout << "- After Night, a new day begins, and weeks accumulate.\n\n";
    std::cout << "Currency System:\n";
    std::cout << "- 100 Copper = 1 Silver\n";
    std::cout << "- 100 Silver = 1 Gold\n";
    std::cout << "- 100 Gold = 1 Platinum\n\n";
    std::cout << "Sleep Mechanic:\n";
    std::cout << "- If not slept, reduce health by ~5% at the start of each new day.\n";
    std::cout << "- Sleep at the Tavern to restore HP and avoid the penalty.\n\n";
    std::cout << "Other Mechanics:\n";
    std::cout << "- Explore to fight enemies or encounter events.\n";
    std::cout << "- Travel to discover new locations.\n";
    std::cout << "- Manage your party, inventory, and stats.\n";
    std::cout << "- Visit stores, taverns, and magic shops in towns.\n\n";
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void characterCreated() {
    std::cout << "\nCharacter created successfully!" << std::endl;
}

void stepIntoGrey() {
    std::cout << "\nPress Enter to step into the grey...\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void acceptFatePrompt() {
    std::cout << "\nAccept this fate? (y/n): ";
}

} // namespace display
