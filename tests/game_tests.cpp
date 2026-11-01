#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "app/session.hpp"
#include "combat/abilities.hpp"
#include "combat/combat.hpp"
#include "data/class.hpp"
#include "data/race.hpp"
#include "systems/save.hpp"
#include "world/encounters.hpp"
#include "ui/player_controller.hpp"

namespace {
void check(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
Player player(const std::string& className = "Warrior") {
    PlayerClassCollection classes;
    PlayerRaceDatabase races;
    Player result("Tester", {races.templates[0].name, {races.templates[0].lore.description}},
                  classes.getClassTemplate(className));
    result.applyRaceBonus(races.templates[0].statBonus);
    return result;
}
Enemy enemy() {
    Enemy result{};
    result.name = "Training foe";
    result.stats.data.hitpoints = result.stats.data.maxHitpoints = 10000;
    result.stats.data.attack = 4;
    result.stats.data.critDamage = 1;
    return result;
}
std::string contents(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}
void write(const std::filesystem::path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary); out << text;
}

void persistence(const std::filesystem::path& directory) {
    auto file = directory / "roundtrip.dat";
    auto second = directory / "second.dat";
    std::string error;
    GameSession session(player("Mage"));
    auto& p = session.hero;
    p.name = "Hero \"quoted\" \\ path\nsecond line";
    p.economy = {2, 3, 4, 5};
    p.progression = {3, 123.25f, 2};
    p.progression.experienceRequired = 144.0f;
    p.stats.expe = 12.125f;
    p.timeSystem.setState(34, TimeSystem::TimePeriod::Evening, 8);
    p.sleptToday = true;
    p.currentLocation = "Enchanted Forest";
    p.currentLocationType = Terrain;
    p.debuffs = {"Poison"};
    p.equippedWeaponDebuffs = {"Bleed"};
    p.equippedWeaponDebuffChance = 0.25f;
    p.learnedSpells = {session.inventory.spellDB.getSpells().front().spellName};
    p.equippedStaffSpells = p.learnedSpells;
    p.defeatedEnemies = {"Rat"};
    p.discoveredLocations = {"Enchanted Forest"};
    p.boughtWeapons = {session.inventory.equipmentDB.getEquipment().front().name};
    p.hiredSpecialCharacters = {"Companion"};
    p.encounteredEvents = {"Event"};
    p.hasNewDictionaryEntry = true;
    session.actionCounter = 3;
    session.lastWeekPaid = 1;
    session.travel.discovered = {"Enchanted Forest", "Peaceful Town"};
    session.travel.marked = {"Peaceful Town"};
    session.inventory.addItem(PlayerInventory::ItemType::Equipment, 0, 1);
    session.inventory.inventory.front().enchanted = true;
    session.inventory.equipped.armorIndex = 0;
    p.stats.armor += 5;
    session.inventory.addItem(PlayerInventory::ItemType::Equipment, 5, 1);
    session.inventory.equipped.weaponIndex = 5;
    p.stats.attack += 22;
    session.inventory.addItem(PlayerInventory::ItemType::Equipment, 10, 1);
    session.inventory.equipped.staffIndex = 10;
    p.stats.magicAttack += 22;
    session.inventory.addItem(PlayerInventory::ItemType::Potion, 0, 3);
    session.inventory.addItem(PlayerInventory::ItemType::FoodAndDrink, 0, 2);
    // Effective stats already include this buff. Loading must not add it again.
    session.inventory.activeBuff.emplace("Test buff", 2, 0, 3, 1, 4, 0.1f, 0.2f, 2);
    p.stats.attack += 2;
    p.stats.magicAttack += 3;
    p.stats.magicArmor += 1;
    p.stats.maxMana += 4;
    p.stats.critRate += 0.1f;
    p.stats.critDamage += 0.2f;
    NPC npc{};
    npc.name = "Companion";
    npc.stats = p.stats;
    npc.race = p.race;
    npc.playerClass = p.playerClass;
    npc.level = 3;
    npc.inventory = {"Small Health Potion"};
    npc.equippedWeapon = "Sword";
    npc.equippedArmor = "Armor";
    npc.debuffs = {"Poison"};
    npc.spells = p.learnedSpells;
    npc.wagePerWeek = 16;
    npc.story = "A tale\nwith \"quotes\".";
    npc.dialogues = {"Hello!", "A second line."};
    session.party.push_back(npc);
    // Existing special-NPC generation can produce matching display names.
    // They remain distinct companion records and must both survive a save.
    npc.story = "Another companion with the same name.";
    session.party.push_back(npc);
    check(saves::save(session, file, error), "Save failed: " + error);
    auto loaded = saves::load(file, error);
    check(loaded.has_value(), "Load failed: " + error);
    check(saves::save(*loaded, second, error), "Resave failed: " + error);
    check(contents(file) == contents(second), "Full session did not round-trip exactly");
    check(loaded->hero.stats.magicAttack == p.stats.magicAttack, "Buff applied twice on load");
    check(loaded->hero.stats.maxMana == p.stats.maxMana, "Max mana changed on load");
    check(loaded->hero.timeSystem.getTotalWeeks() == 1 && loaded->lastWeekPaid == 1 &&
          loaded->actionCounter == 3, "Time/wage bookkeeping lost");
    PlayerController controller(loaded->hero, loaded->inventory);
    loaded->hero.stats.expe = 120;
    int level = loaded->hero.stats.level;
    controller.levelUpChecker();
    check(loaded->hero.stats.level == level, "Loaded XP threshold reset to level 1");
    loaded->inventory.tickBuffs(loaded->hero);
    check(loaded->inventory.activeBuff->remainingTurns == 1, "Buff lifetime not restored");
    loaded->inventory.tickBuffs(loaded->hero);
    check(!loaded->inventory.activeBuff, "Buff did not expire");
    check(loaded->hero.stats.magicAttack == p.stats.magicAttack - 3, "Expired magic buff leaked attack");
    check(std::abs(loaded->hero.stats.magicArmor - (p.stats.magicArmor - 1)) < 0.00001f,
          "Expired magic buff leaked armor");

    const auto good = contents(file);
    write(second, good.substr(0, good.size() / 2));
    check(!saves::load(second, error), "Truncated save accepted");
    write(second, "RPGGAME 99\n" + good.substr(good.find('\n') + 1));
    check(!saves::load(second, error), "Unsupported version accepted");
    write(second, good + "garbage");
    check(!saves::load(second, error), "Trailing data accepted");
    check(!saves::load(directory / "missing.dat", error), "Missing save accepted");
    GameSession invalid = session;
    invalid.inventory.inventory.front().dbIndex = 999999;
    check(!saves::save(invalid, file, error), "Invalid item index accepted");
    check(contents(file) == good, "Failed save destroyed previous slot");
    check(!saves::save(session, directory / "missing" / "save.dat", error), "Unwritable path accepted");
    // A real write/replacement failure must also preserve its destination.
    auto occupied = directory / "occupied";
    std::filesystem::create_directory(occupied);
    write(occupied / "keep", "sentinel");
    check(!saves::save(session, occupied, error), "Replaced a directory with a save");
    check(contents(occupied / "keep") == "sentinel", "Failed replacement lost destination");
    session.hero.economy.gold += 1;
    check(saves::save(session, file, error), "Cannot overwrite an existing slot");
    loaded = saves::load(file, error);
    check(loaded && loaded->hero.economy.gold == 4, "Overwrite did not persist");

    // Corrupt a numeric field in a clean save, preserving the rest of the format.
    GameSession simple(player());
    check(saves::save(simple, second, error), "Simple save failed");
    auto corrupt = contents(second);
    size_t hp = corrupt.find('\n', corrupt.find('\n') + 1) + 1;
    size_t hpEnd = corrupt.find('\n', hp);
    corrupt.replace(hp, hpEnd - hp, "2147483647");
    write(second, corrupt);
    check(!saves::load(second, error), "Invalid saved HP accepted");
}

void abilities() {
    PlayerClassCollection classes;
    std::vector<NPC> party;
    for (const auto& cls : classes.templates) {
        auto p = player(cls.name);
        auto foe = enemy();
        p.stats.hitpoints /= 2;
        p.stats.mana /= 2;
        AbilityState state;
        std::string log;
        check(useSignature(p, party, foe, state, 0.99f, 0.1f, log), "Ability failed: " + cls.name);
        check(state.used && !log.empty(), "Missing ability use/log");
        int hp = p.stats.hitpoints, mana = p.stats.mana, enemyHP = foe.stats.data.hitpoints;
        check(!useSignature(p, party, foe, state, 0.99f, 0.1f, log), "Ability used twice");
        check(p.stats.hitpoints == hp && p.stats.mana == mana && foe.stats.data.hitpoints == enemyHP,
              "Unavailable ability consumed resources");
        check(p.stats.hitpoints <= p.stats.maxHitpoints && p.stats.mana <= p.stats.maxMana, "Restore exceeded cap");
        AbilityState nextFight;
        check(useSignature(p, party, foe, nextFight, 0.99f, 0.1f, log), "Ability not available next encounter");
    }
    auto p = player("Berserker");
    p.stats.hitpoints = 1;
    auto foe = enemy();
    AbilityState state;
    std::string log;
    useSignature(p, party, foe, state, 0.99f, 0.1f, log);
    check(p.stats.hitpoints == 1, "Blood Rage killed its user");
    p = player("Knight"); state = {};
    float armor = p.stats.armor;
    useSignature(p, party, foe, state, 0.99f, 0.1f, log);
    check(state.consumeRetaliation() == 0.5f && state.consumeRetaliation() == 1.0f, "Brace lasted wrong number of turns");
    check(p.stats.armor == armor, "Brace permanently modified stats");
    p = player("Shrewd Diplomat"); state = {};
    useSignature(p, party, foe, state, 0.99f, 0.1f, log);
    check(state.consumeRetaliation() == 0 && state.consumeRetaliation() == 1, "Distract lasted wrong number of turns");
    p = player("Ranger"); state = {};
    foe.stats.data.dodgeRate = 1;
    int hp = foe.stats.data.hitpoints;
    useSignature(p, party, foe, state, 0, 0, log);
    check(foe.stats.data.hitpoints < hp, "Aimed Shot was dodged");
    p = player("Sorcerer"); state = {};
    p.stats.mana = 11;
    useSignature(p, party, foe, state, 0, 0, log);
    check(p.stats.mana == 6, "Mana Burst cost incorrect");
    p = player("Sheltered Noble"); state = {};
    NPC living{}, dead{};
    living.stats = p.stats; living.stats.hitpoints = 1;
    dead.stats = p.stats; dead.stats.hitpoints = 0;
    party = {living, dead};
    useSignature(p, party, foe, state, 0, 0, log);
    check(party[0].stats.hitpoints > 1 && party[1].stats.hitpoints == 0, "Rally healed wrong targets");
    p.stats.hitpoints = 0; state = {};
    check(!useSignature(p, party, foe, state, 0, 0, log) && !state.used, "Dead player used ability");
}

void encounters() {
    EnemyController controller;
    locationDatabase locations;
    auto cell = currentArea("Abandoned Cell");
    check(cell.type == Terrain && cell.difficultyLevel == 1, "Starting area incorrect");
    EnemyDatabase database;
    for (const auto& area : locations.getLocations()) {
        auto resolved = currentArea(area.name);
        check(resolved.difficultyLevel == area.difficultyLevel && resolved.type == area.type, "Area lookup lost difficulty/type");
        bool eligible = false;
        for (const auto& entry : database.templates)
            if (entry.difficultyLevel <= area.difficultyLevel)
                for (auto type : entry.allowedLocations) if (type == area.type) eligible = true;
        check(controller.hasEligibleEnemy(area.difficultyLevel, area.type) == eligible, "Eligibility mismatch");
        if (eligible) {
            auto spawned = controller.encounterEnemy(area.difficultyLevel, area.type);
            bool valid = false;
            for (const auto& entry : database.templates) if (entry.name == spawned.name) {
                for (auto type : entry.allowedLocations)
                    if (type == area.type && entry.difficultyLevel <= area.difficultyLevel) valid = true;
            }
            check(valid, "Wrong enemy pool for location");
        }
    }
    check(!controller.hasEligibleEnemy(0, Dungeon), "Empty enemy pool not detected");
    check(encounterChance(Terrain) == 0.6f && encounterChance(Dungeon) == 0.8f &&
          encounterChance(PeacefulTown) == 0.3f && encounterChance(PeacefulVillage) == 0.2f &&
          encounterChance(SpellStore) == 0.5f, "Incorrect encounter probabilities");
}

void combatMenus() {
    for (bool learned : {false, true}) {
        auto p = player("Knight");
        p.stats.hitpoints = p.stats.maxHitpoints = 1000;
        auto foe = enemy();
        std::vector<NPC> party;
        PlayerInventory inventory;
        NPCGenerator generator;
        SpellDatabase spells;
        if (learned) p.learnedSpells.push_back(spells.getSpells().front().spellName);
        CombatScreen screen(p, party, foe, p.timeSystem, generator, spells);
        CombatSystem combat;
        // Use ability, try again (no turn), then flee. Exactly one retaliation.
        std::istringstream input(learned ? "4\n\n4\n\n5\n1\n\n3\n" : "4\n\n4\n\n3\n");
        std::ostringstream output;
        auto* oldIn = std::cin.rdbuf(input.rdbuf());
        auto* oldOut = std::cout.rdbuf(output.rdbuf());
        screen.startCombat(combat, inventory);
        std::cin.rdbuf(oldIn); std::cout.rdbuf(oldOut);
        check(output.str().find("4. Brace") != std::string::npos, "Signature menu missing");
        check((output.str().find("5. Cast Spell") != std::string::npos) == learned, "Spell menu dispatch wrong");
        check(output.str().find("unavailable this encounter") != std::string::npos, "Used ability not rejected");
        check(p.stats.hitpoints >= (learned ? 993 : 997), "Unavailable ability spent an extra turn");
        if (learned) {
            check(output.str().find("cast Glintstone Pebble") != std::string::npos, "Spell action dispatched incorrectly");
            check(p.stats.mana == 0, "Spell did not consume mana");
        }
    }
}
}

int main(int argc, char** argv) {
    try {
        check(argc >= 2, "Pass test group");
        std::string group = argv[1];
        if (group == "save") {
            check(argc == 3, "Pass scratch directory");
            auto directory = std::filesystem::path(argv[2]) / "save-tests";
            std::filesystem::create_directories(directory);
            persistence(directory);
        } else if (group == "abilities") abilities();
        else if (group == "encounters") encounters();
        else if (group == "combat") combatMenus();
        else throw std::runtime_error("Unknown test group");
        std::cout << "Passed " << group << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
