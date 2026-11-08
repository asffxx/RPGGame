#include "display.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>

#include "core/ui.hpp"
#include "catalogs/class_skill.hpp"

namespace display {

// Round display values to two decimals and omit unnecessary trailing zeros.
std::string num(float value) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(2) << value;
    std::string text = output.str();
    if (text.find('.') != std::string::npos) {
        while (!text.empty() && text.back() == '0') text.pop_back();
        if (!text.empty() && text.back() == '.') text.pop_back();
    }
    return text;
}

// --- Shared prompts (core/ui) ---
void pressEnter() {
    std::cout << "\nPress Enter to continue...";
    try {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } catch (const std::ios_base::failure&) {
        // Input stream hit EOF (test scripts and pipes); strip failbit so
        // callers can finish cleanly. eofbit stays visible.
        std::cin.clear(std::cin.rdstate() & ~std::ios::failbit);
    }
}

// --- Time system (core/timesystem) ---
void sleepPenalty() {
    displayBorderedMenu({"[ Sleep Deprived ]", "You fell face down, thats a bummer."}, "");
    pressEnter();
}

// --- Enemies (entities/enemy) ---
void enemyAppeared(const Enemy& enemy) {
    displayBorderedMenu({"[ Encounter ]", "A wild " + enemy.name + " has appeared!"}, "");
    pressEnter();
}

void spoils(const std::vector<std::string>& lines) {
    displayBorderedMenu(lines, "");
    pressEnter();
}

std::string coins(int platinum, int gold, int silver, int copper) {
    std::string text;
    auto add = [&text](int value, const char* suffix) {
        if (!value) return;
        if (!text.empty()) text += " ";
        text += std::to_string(value) + suffix;
    };
    add(platinum, "p");
    add(gold, "g");
    add(silver, "s");
    add(copper, "c");
    return text.empty() ? "0c" : text;
}



// --- Combat (combat/combat) ---
void combatRunAway() {
    displayBorderedMenu({"[ Escape ]", "You ran away!"}, "");
    pressEnter();
}

void combatInvalidChoice() {
    std::cout << "Invalid choice!" << std::endl;
}

void combatSpellPrompt(const std::vector<std::string>& spellNames) {
    std::vector<std::string> lines = {"[ Cast a Spell ]"};
    for (size_t i = 0; i < spellNames.size(); ++i)
        lines.push_back(std::to_string(i + 1) + ". " + spellNames[i]);
    displayBorderedMenu(lines, "Choose a spell: ");
}

void combatManaInsufficient() {
    displayBorderedMenu({"[ Cast a Spell ]", "Mana insufficient."}, "");
    pressEnter();
}

void combatDefeat(const std::string& playerName) {
    displayBorderedMenu({"[ Defeat ]", playerName + " has been defeated!"}, "");
    pressEnter();
}

// --- Inventory (systems/inventory) ---
bool usePotionPrompt(const std::string& potionName) {
    displayBorderedMenu({"[ " + potionName + " ]", "1. Drink it", "2. Leave it"}, "Choose: ");
    return getNumberInput(1, 2) == 1;
}

void potionRecoveredHp(int heal) {
    displayBorderedMenu({"[ Potion ]", "Recovered " + std::to_string(heal) + " HP."}, "");
    pressEnter();
}

void potionRecoveredMana(int restore) {
    displayBorderedMenu({"[ Potion ]", "Recovered " + std::to_string(restore) + " Mana."}, "");
    pressEnter();
}

void potionEffect(const std::string& effectDesc) {
    displayBorderedMenu({"[ Potion ]", effectDesc}, "");
    pressEnter();
}

bool useFoodPrompt(const std::string& foodName) {
    displayBorderedMenu({"[ " + foodName + " ]", "1. Eat it", "2. Leave it"}, "Choose: ");
    return getNumberInput(1, 2) == 1;
}

void foodRecoveredHp(int heal) {
    displayBorderedMenu({"[ Meal ]", "Recovered " + std::to_string(heal) + " HP."}, "");
    pressEnter();
}

void foodRecoveredMana(int restore) {
    displayBorderedMenu({"[ Meal ]", "Recovered " + std::to_string(restore) + " Mana."}, "");
    pressEnter();
}

void foodMaxHpIncrease(int amount) {
    displayBorderedMenu({"[ Meal ]", "Max HP increased by " + std::to_string(amount) + "."}, "");
    pressEnter();
}

void foodMaxManaIncrease(int amount) {
    displayBorderedMenu({"[ Meal ]", "Max Mana increased by " + std::to_string(amount) + "."}, "");
    pressEnter();
}

bool unequipPrompt(const std::string& itemName) {
    displayBorderedMenu({"[ " + itemName + " ]", "1. Unequip it", "2. Keep it"}, "Choose: ");
    return getNumberInput(1, 2) == 1;
}

void itemUnequipped(const std::string& itemName) {
    displayBorderedMenu({"[ Backpack ]", itemName + " unequipped."}, "");
    pressEnter();
}

bool equipPrompt(const std::string& itemName) {
    displayBorderedMenu({"[ " + itemName + " ]", "1. Equip it", "2. Leave it"}, "Choose: ");
    return getNumberInput(1, 2) == 1;
}

void itemEffect(const std::string& effectDesc) {
    displayBorderedMenu({"[ Equipment ]", effectDesc}, "");
    pressEnter();
}

// --- Events (systems/event) ---
void eventBox(const std::vector<std::string>& lines) {
    displayBorderedMenu(lines, "");
}

void eventPressEnter() {
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// --- Party (systems/party) ---
void partyEmpty() {
    displayBorderedMenu({"[ Party ]", "Your party is empty."}, "");
    pressEnter();
}

void partyMemberLeft(const std::string& name) {
    displayBorderedMenu({"[ Party ]", "Due to insufficient gold, " + name + " has left the party."}, "");
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
    displayBorderedMenu({"[ " + name + " ]", "What do you want to do?", "1. Talk to member",
                         "2. What is your story?", "3. Kick member", "4. Cancel"}, "Choose: ");
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

// --- Dictionary (systems/dictionary) ---
void dictionaryEmptyEnemies() {
    displayBorderedMenu({"[ Dictionary ]", "No defeated enemies yet."}, "");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEmptyWeapons() {
    displayBorderedMenu({"[ Dictionary ]", "No bought weapons yet."}, "");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEmptyLocations() {
    displayBorderedMenu({"[ Dictionary ]", "No discovered locations yet."}, "");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEmptyEvents() {
    displayBorderedMenu({"[ Dictionary ]", "No encountered events yet."}, "");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEmptyCharacters() {
    displayBorderedMenu({"[ Dictionary ]", "No hired special characters yet."}, "");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void dictionaryEnemyHeader(const std::string& name) {
    displayBorderedMenu({"[ " + name + " ]", "1. Stats", "2. Description", "3. Exit"}, "Choose: ");
}

void dictionaryEnemyStats(const EnemyDatabase::EnemyTemplate& tmpl) {
    displayBorderedMenu({"[ Stats ]",
        "HP: " + std::to_string(tmpl.stats.data.hitpoints) + "/" + std::to_string(tmpl.stats.data.maxHitpoints),
        "Attack: " + std::to_string(tmpl.stats.data.attack),
        "Magic Attack: " + std::to_string(tmpl.stats.data.magicAttack),
        "Armor: " + num(tmpl.stats.data.armor),
        "Magic Armor: " + num(tmpl.stats.data.magicArmor),
        "Level: " + std::to_string(tmpl.stats.data.level),
        "Experience: " + num(tmpl.stats.data.expe)}, "");
    pressEnter();
}

void dictionaryEnemyDescription(const std::string& desc) {
    displayBorderedMenu({"[ Description ]", desc}, "");
    pressEnter();
}

void dictionaryWeaponHeader(const std::string& name) {
    displayBorderedMenu({"[ " + name + " ]", "1. Stats", "2. Description", "3. Exit"}, "Choose: ");
}

void dictionaryWeaponStats(const EquipmentandWeaponDatabase::equipmentProperties& eq) {
    displayBorderedMenu({"[ Stats ]",
        "Type: " + eq.type,
        "Attack Increase: " + std::to_string(eq.attackIncrease),
        "Magic Attack Increase: " + std::to_string(eq.magicAttackIncrease),
        "Crit Rate Increase: " + num(eq.critRateIncrease),
        "Crit Damage Increase: " + num(eq.critDamageIncrease),
        "Price: " + std::to_string(eq.pricePlatinum) + "p " + std::to_string(eq.priceGold) + "g " +
            std::to_string(eq.priceSilver) + "s " + std::to_string(eq.priceCopper) + "c"}, "");
    pressEnter();
}

void dictionaryWeaponDescription(const std::string& desc) {
    displayBorderedMenu({"[ Description ]", desc}, "");
    pressEnter();
}

void dictionaryLocationHeader(const std::string& name) {
    displayBorderedMenu({"[ " + name + " ]", "1. Stats", "2. Description", "3. Exit"}, "Choose: ");
}

void dictionaryLocationStats(const locationDatabase::locationProperties& loc) {
    std::string typeStr;
    switch (loc.type) {
        case PeacefulVillage: typeStr = "Peaceful Village"; break;
        case PeacefulTown: typeStr = "Peaceful Town"; break;
        case Dungeon: typeStr = "Dungeon"; break;
        case Terrain: typeStr = "Terrain"; break;
        case SpellStore: typeStr = "Spell Store"; break;
        case PeacefulCity: typeStr = "Peaceful City"; break;
    }
    displayBorderedMenu({"[ Stats ]",
        "Difficulty Level: " + std::to_string(loc.difficultyLevel),
        "Type: " + typeStr}, "");
    pressEnter();
}

void dictionaryLocationDescription(const std::string& desc) {
    displayBorderedMenu({"[ Description ]", desc}, "");
    pressEnter();
}

void dictionaryEventHeader(const std::string& name) {
    displayBorderedMenu({"[ " + name + " ]", "1. Stats", "2. Description", "3. Exit"}, "Choose: ");
}

void dictionaryEventStats(const eventDatabase::eventProperties& ev) {
    std::vector<std::string> lines = {"[ Stats ]",
        "Gold Reward: " + std::to_string(ev.goldReward),
        "Exp Reward: " + std::to_string(ev.expReward),
        "Health Effect: " + std::to_string(ev.healthEffect),
        "Mana Effect: " + std::to_string(ev.manaEffect),
        std::string("Enemy Encounter: ") + (ev.enemyEncounter ? "Yes" : "No")};
    if (ev.enemyEncounter) {
        lines.push_back("Enemy: " + ev.enemyName);
        lines.push_back("Count: " + std::to_string(ev.enemyCount));
    }
    displayBorderedMenu(lines, "");
    pressEnter();
}

void dictionaryEventDescription(const std::string& desc) {
    displayBorderedMenu({"[ Description ]", desc}, "");
    pressEnter();
}

void dictionarySpecialCharacters(const std::vector<std::string>& names) {
    std::vector<std::string> lines = {"[ Special Characters ]"};
    for (const auto& name : names) lines.push_back("- " + name);
    displayBorderedMenu(lines, "");
    pressEnter();
}

// --- Player controller (ui/player_controller) ---
void showPlayerStats(const Player& player, float reqAmount, const std::optional<PlayerInventory::Buff>& activeBuff) {
    std::vector<std::string> lines = {
        "[ Player Stats ]",
        "Hitpoints: " + std::to_string(player.stats.hitpoints) + "/" + std::to_string(player.stats.maxHitpoints),
        "----------------------------",
    };
    if (activeBuff) {
        lines.push_back("Armor: " + num(player.stats.armor * 100) + "%" +
                        " ( + " + std::to_string(activeBuff->defenseBonus) + " from a potion, (" +
                        std::to_string(activeBuff->remainingTurns) + " turns left))");
        lines.push_back("Magic Armor: " + num(player.stats.magicArmor * 100) + "%" +
                        " ( + " + std::to_string(activeBuff->magicDefenseBonus) + " from a potion, (" +
                        std::to_string(activeBuff->remainingTurns) + " turns left))");
        lines.push_back("Attack: " + std::to_string(player.stats.attack) +
                        " ( + " + std::to_string(activeBuff->attackBonus) + " from a potion, (" +
                        std::to_string(activeBuff->remainingTurns) + " turns left))");
        lines.push_back("Magic Attack: " + std::to_string(player.stats.magicAttack) +
                        " ( + " + std::to_string(activeBuff->magicAttackBonus) + " from a potion, (" +
                        std::to_string(activeBuff->remainingTurns) + " turns left))");
    } else {
        lines.push_back("Armor: " + num(player.stats.armor * 100) + "%");
        lines.push_back("Magic Armor: " + num(player.stats.magicArmor * 100) + "%");
        lines.push_back("Attack: " + std::to_string(player.stats.attack));
        lines.push_back("Magic Attack: " + std::to_string(player.stats.magicAttack));
    }
    lines.push_back("Level: " + std::to_string(player.stats.level));
    lines.push_back("Experience: " + num(player.stats.expe) + "/" + num(reqAmount));
    lines.push_back("Currency: " + std::to_string(player.economy.platinum) + "p " + std::to_string(player.economy.gold) + "g " +
                    std::to_string(player.economy.silver) + "s " + std::to_string(player.economy.copper) + "c");
    displayBorderedMenu(lines, "");
    pressEnter();
}

void leveledUp() {
    displayBorderedMenu({"[ Level Up ]", "You leveled Up!", "Your stats have been increased by 10%."}, "");
}

// --- Establishments (world/establishments) ---
void tavernFoodPurchased(const std::string& name) {
    message("You purchased " + name + ". It smells... edible.");
}

void tavernFoodTooExpensive() {
    message("You don't have enough coin for that.");
}

void tavernPartyTooBig() {
    message("'Your group is too big already,' the Tavernkeeper remarks. (Max 4 members)");
}

void tavernHireDeclined() {
    message("You decide not to hire them.");
}

void tavernHireTooExpensive() {
    message("You realize you can't afford their services.");
}

void tavernHireJoined(const NPC& npc, const std::string& line) {
    displayBorderedMenu({"[ " + npc.name + " Joins ]", npc.name + " stands up and joins your cause!", line}, "");
    pressEnter();
}

void storeBoughtPotion(const std::string& name) {
    message("Bought " + name + "!");
}

void storeBoughtEquipment(const std::string& name) {
    message("Bought " + name + "!");
}

void storeSold(const std::string& name, int quantity, long long copper) {
    message("Sold " + name + " x" + std::to_string(quantity) + " for " + std::to_string(copper) + " copper.");
}

void storeNothingToSell() {
    message("You have nothing to sell.");
}

void storeNotEnoughCurrency() {
    message("Not enough currency!");
}

void magicStoreNoSpells() {
    message("No spells available for your level.");
}

void magicStoreLearned(const std::string& name) {
    message("Learned " + name + "!");
}

void magicStoreAlreadyKnown(const std::string& name) {
    message("You already know " + name + "!");
}

void magicStoreNoEnchantable() {
    message("No equipped enchantable items.");
}

void magicStoreEnchantChoice(const std::vector<std::string>& lines) {
    std::vector<std::string> options = {"[ Enchant Equipment ]"};
    for (size_t i = 0; i < lines.size(); ++i)
        options.push_back(std::to_string(i + 1) + ". " + lines[i]);
    options.push_back("0. Cancel");
    displayBorderedMenu(options, "Choose item to enchant: ");
}

void magicStoreNotEnoughGold() {
    message("Not enough gold! Enchanting costs 100 gold.");
}

void magicStoreEnchanted(const std::string& itemName, const std::string& applied) {
    message("Enchanted " + itemName + " with " + applied + "!");
}

// --- Travel (world/travel) ---
void travelNoMarkedLocations() {
    message("No marked locations.");
}

void travelLocationList(const std::vector<std::string>& lines) {
    displayBorderedMenu(lines, "Choose a location: ");
}

void travelLocationHeader(const std::string& name, const std::string& description, bool hasQuest) {
    std::vector<std::string> lines = {
        "[ " + name + " ]",
        description,
        "----------------------------",
        "1. Explore",
        "2. Mark location",
        "3. Leave",
    };
    if (hasQuest) lines.push_back("4. Quest Action (explicit quest fights can occur on safe routes)");
    displayBorderedMenu(lines, "Choose an action: ");
}

void travelLocationMarked() {
    message("Location marked for safe travel.");
}

// --- Main menu / character creation (app) ---
void gameBasics() {
    std::vector<std::string> lines = {
        "[ Game Basics ]",
        "Time System:",
        "- 4 turns advance the state of the day (Morning -> Afternoon -> Evening -> Night).",
        "- After Night, a new day begins, and weeks accumulate.",
        "----------------------------",
        "Currency System:",
        "- 100 Copper = 1 Silver",
        "- 100 Silver = 1 Gold",
        "- 100 Gold = 1 Platinum",
        "----------------------------",
        "Sleep Mechanic:",
        "- If not slept, reduce health by ~5% at the start of each new day.",
        "- Sleep at the Tavern to restore HP and avoid the penalty.",
        "----------------------------",
        "Other Mechanics:",
        "- The main menu groups actions by category; choose one, then an action (0 to go back).",
        "- Save Game writes savegame.dat in the working directory; Continue loads it.",
        "- Saving is manual, free, and overwrites the slot. Exit does not save.",
        "- Each class has a signature ability usable once per encounter (action 4).",
        "- Encounters use your area's difficulty/type; marked routes avoid combat.",
        "- Peaceful cities have stores, taverns, magic stores, and a Guild Hall.",
        "- Ask a tavern for Rumors, then use Travel -> Follow a lead to find cities.",
        "- Join all five guilds if you like; each offers two sequential quests.",
        "- Guild Hall options 3/4 offer Side Quests and Commissions, including after its storyline.",
        "- Boards explain level, membership, and guild-quest turn-in requirements.",
        "- Side quests are one-time; commissions require accepting a fresh run after each turn-in.",
        "- Read your Quest Journal, perform Quest Actions at their targets, then return for rewards.",
        "- Explicit Quest Actions can start fights even on marked safe routes.",
        "- Explore to fight enemies or encounter events.",
        "- Travel to discover new locations.",
        "- Manage your party, inventory, and stats.",
        "- Visit stores, taverns, and magic shops in towns.",
    };
    displayBorderedMenu(lines, "");
    pressEnter();
}

void message(const std::string& text) {
    displayBorderedMenu({"[ Notice ]", text}, "");
    pressEnter();
}

void message(const std::vector<std::string>& lines) {
    displayBorderedMenu(lines, "");
    pressEnter();
}

namespace {

std::vector<std::string> creationStats(const Stats& stats) {
    return {
        "HP: " + std::to_string(stats.hitpoints) + "/" + std::to_string(stats.maxHitpoints) +
            "   Mana: " + std::to_string(stats.mana) + "/" + std::to_string(stats.maxMana),
        "Attack: " + std::to_string(stats.attack) + "   Magic attack: " + std::to_string(stats.magicAttack),
        "Armor: " + num(stats.armor * 100) + "%   Magic armor: " + num(stats.magicArmor * 100) + "%",
        "Critical chance: " + num(stats.critRate * 100) + "%   Critical multiplier: " + num(stats.critDamage),
        "Dodge: " + num(stats.dodgeRate * 100) + "%   Magic amplifier: " + num(stats.magicAmplifierRate),
    };
}

} // namespace

void racePreview(const PlayerRaceTemplate& race) {
    const auto& s = race.statBonus;
    std::vector<std::string> lines = {
        "[ " + race.name + " ]",
        race.lore.description,
        "----------------------------",
        "Stat bonuses (added to class):",
        "HP: +" + std::to_string(s.maxHitpoints) + "   Mana: +" + std::to_string(s.maxMana),
        "Attack: +" + std::to_string(s.attack) + "   Magic attack: +" + std::to_string(s.magicAttack),
        "Armor: +" + num(s.armor * 100) + "%   Magic armor: +" + num(s.magicArmor * 100) + "%",
        "----------------------------",
        "1. Confirm race",
        "2. Back",
    };
    displayBorderedMenu(lines, "Choose an option: ");
}

void classPreview(const PlayerClassTemplate& playerClass, const PlayerRaceTemplate& race) {
    std::vector<std::string> lines = {
        "[ " + playerClass.name + " ]",
        playerClass.lore.description,
        "----------------------------",
        "Base stats:",
    };
    const auto base = creationStats(playerClass.stats);
    lines.insert(lines.end(), base.begin(), base.end());
    lines.push_back("----------------------------");
    Player preview("", {race.name, {race.lore.description}}, playerClass);
    preview.applyRaceBonus(race.statBonus);
    lines.push_back("Starting stats with " + race.name + ":");
    const auto started = creationStats(preview.stats);
    lines.insert(lines.end(), started.begin(), started.end());
    const auto& ability = signatureAbility(playerClass.name);
    lines.push_back("----------------------------");
    lines.push_back(std::string("Signature: ") + ability.name);
    lines.push_back(ability.description);
    lines.push_back("Available from level 1, once per encounter; uses your turn.");
    lines.push_back("----------------------------");
    lines.push_back("1. Confirm class");
    lines.push_back("2. Back");
    displayBorderedMenu(lines, "Choose an option: ");
}

std::string characterName() {
    std::string name;
    do {
        displayBorderedMenu({"[ New Character ]", "Choose a name for your character."}, "Enter your name: ");
        if (!std::getline(std::cin, name)) return "Adventurer";
    } while (name.find_first_not_of(" \t\r") == std::string::npos);
    return name;
}
} // namespace display
