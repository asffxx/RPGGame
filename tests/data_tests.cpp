#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

#include "app/session.hpp"
#include "core/ids.hpp"
#include "catalogs/class.hpp"
#include "catalogs/equipment.hpp"
#include "catalogs/food.hpp"
#include "catalogs/guild.hpp"
#include "catalogs/location.hpp"
#include "catalogs/potion.hpp"
#include "catalogs/race.hpp"
#include "catalogs/spell.hpp"
#include "entities/enemy.hpp"
#include "gameplay/datafile.hpp"
#include "gameplay/save.hpp"

namespace {

void check(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void checkIds(const std::vector<std::string>& list, const std::string& what) {
    std::set<std::string> seen;
    for (const auto& id : list) {
        check(ids::valid(id), what + ": invalid id \"" + id + "\"");
        check(seen.insert(id).second, what + ": duplicate id \"" + id + "\"");
    }
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
}

void writeFile(const std::filesystem::path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    out << text;
}

GameSession newSession() {
    PlayerClassCollection classes;
    PlayerRaceDatabase races;
    PlayerRace race{ races.templates.front().name, { races.templates.front().lore.description } };
    Player hero("Tester", race, classes.getClassTemplate("Warrior"));
    GameSession session(hero);
    session.inventory.addItem(PlayerInventory::ItemType::Equipment, 0, 1);
    session.inventory.addItem(PlayerInventory::ItemType::Potion, 0, 2);
    session.inventory.addItem(PlayerInventory::ItemType::FoodAndDrink, 0, 1);
    return session;
}

void idLayer() {
    check(ids::slug("Iron Sword") == "iron_sword", "slug keeps words");
    check(ids::slug("Dragon's Lair") == "dragon_s_lair", "slug folds punctuation");
    check(ids::slug("  spaced  out  ") == "spaced_out", "slug trims runs");
    check(ids::slug("") == "item", "slug fallback");
    check(ids::slug("3rd Age") == "n_3rd_age", "slug prefixes leading digit");
    std::set<std::string> used;
    check(ids::unique("Mace", used) == "mace", "first id");
    check(ids::unique("Mace", used) == "mace_2", "second id");
    check(ids::unique("Mace", used) == "mace_3", "third id");
    check(ids::valid("snake_case_2"), "valid id");
    check(!ids::valid("BadId"), "uppercase rejected");
    check(!ids::valid("bad-id"), "dash rejected");
    check(!ids::valid("_leading"), "leading underscore rejected");
    check(!ids::valid(""), "empty rejected");

    EquipmentandWeaponDatabase equipment;
    PotionDatabase potions;
    FoodandDrinksDatabase food;
    locationDatabase locations;
    EnemyDatabase enemies;
    PlayerClassCollection classes;
    PlayerRaceDatabase races;
    SpellDatabase spells;
    GuildDatabase guilds;

    std::vector<std::string> equipmentIds, potionIds, foodIds, locationIds;
    for (const auto& item : equipment.getEquipment()) equipmentIds.push_back(item.id);
    for (const auto& item : potions.getPotions()) potionIds.push_back(item.id);
    for (const auto& item : food.getFoodAndDrink()) foodIds.push_back(item.id);
    for (const auto& item : locations.getLocations()) locationIds.push_back(item.id);
    checkIds(equipmentIds, "equipment");
    checkIds(potionIds, "potions");
    checkIds(foodIds, "food");
    checkIds(locationIds, "locations");

    std::vector<std::string> enemyIds, classIds, raceIds, spellIds, guildIds;
    for (const auto& item : enemies.templates) enemyIds.push_back(item.id);
    for (const auto& item : classes.templates) classIds.push_back(item.id);
    for (const auto& item : races.templates) raceIds.push_back(item.id);
    for (const auto& item : spells.getSpells()) spellIds.push_back(item.id);
    for (const auto& item : guilds.getGuilds()) guildIds.push_back(item.id);
    checkIds(enemyIds, "enemies");
    checkIds(classIds, "classes");
    checkIds(raceIds, "races");
    checkIds(spellIds, "spells");
    checkIds(guildIds, "guilds");

    for (size_t i = 0; i < equipmentIds.size(); ++i)
        check(equipment.indexOf(equipmentIds[i]) == static_cast<int>(i), "equipment lookup by id");
    for (size_t i = 0; i < potionIds.size(); ++i)
        check(potions.indexOf(potionIds[i]) == static_cast<int>(i), "potion lookup by id");
    for (size_t i = 0; i < foodIds.size(); ++i)
        check(food.indexOf(foodIds[i]) == static_cast<int>(i), "food lookup by id");
    for (size_t i = 0; i < spellIds.size(); ++i)
        check(spells.indexOf(spellIds[i]) == static_cast<int>(i), "spell lookup by id");
    check(equipment.indexOf("missing_id") == -1, "unknown equipment id");
    check(equipment.find("missing_id") == nullptr, "unknown equipment record");
    check(locations.find(locationIds.front()) != nullptr, "location lookup by id");
    check(enemies.find(enemyIds.front()) != nullptr, "enemy lookup by id");
    check(classes.findById(classIds.front()) != nullptr, "class lookup by id");
    check(races.findById(raceIds.front()) != nullptr, "race lookup by id");

    PlayerInventory inventory;
    for (size_t i = 0; i < equipmentIds.size(); ++i) {
        check(inventory.itemId(PlayerInventory::ItemType::Equipment, i) == equipmentIds[i],
              "inventory exposes catalog ids");
        check(inventory.indexOfItem(PlayerInventory::ItemType::Equipment, equipmentIds[i]) ==
                  static_cast<int>(i), "inventory resolves ids");
    }
    check(inventory.itemId(PlayerInventory::ItemType::Potion, 99999).empty(), "out-of-range id is empty");
    check(inventory.indexOfItem(PlayerInventory::ItemType::Potion, "missing_id") == -1, "unknown item id");
}

void jsonReader(const std::filesystem::path& directory) {
    datafile::Value root;
    std::string error;
    const std::string good = R"({
        "name": "iron_sword",
        "price": 90,
        "ratio": -1.5e2,
        "flags": [true, false, null],
        "nested": {"deep": ["a", "b\u00e9", "\ud83d\ude00"]}
    })";
    check(datafile::parse(good, root, error), "valid json rejected: " + error);
    check(root.at("name").string() == "iron_sword", "string field");
    check(root.at("price").integer() == 90, "integer field");
    check(root.at("ratio").number() == -150.0, "exponent number");
    check(root.at("flags").size() == 3, "array size");
    check(root.at("flags").at(0).boolean(), "boolean field");
    check(root.at("flags").at(2).isNull(), "null field");
    check(root.has("nested"), "object field");
    check(root.at("nested").at("deep").at(2).string() == "\xF0\x9F\x98\x80", "surrogate pair");
    check(!root.has("missing"), "missing field");

    auto rejects = [&](const std::string& text, const std::string& what) {
        datafile::Value value;
        std::string why;
        check(!datafile::parse(text, value, why), "accepted invalid json: " + what);
        check(why.find("line ") == 0 && why.find("column ") != std::string::npos,
              "error lacks position for " + what + ": " + why);
    };
    rejects("{\"a\": 1,}", "trailing comma");
    rejects("[1, 2,]", "trailing array comma");
    rejects("{\"a\": 1, \"a\": 2}", "duplicate key");
    rejects("{\"a\": 01}", "leading zero");
    rejects("{\"a\": 1.}", "empty fraction");
    rejects("{\"a\": .5}", "missing integer part");
    rejects("{\"a\": 1e999}", "overflow");
    rejects("{\"a\": \"unterminated}", "unterminated string");
    rejects("{\"a\": \"\\x41\"}", "invalid escape");
    rejects("{\"a\": \"\\ud800\"}", "lone surrogate");
    rejects("{\"a\": \"tab\tchar\"}", "control character");
    rejects("// comment\n{\"a\": 1}", "comment");
    rejects("{\"a\": 1} trailing", "trailing data");
    rejects("", "empty input");

    datafile::Value fileValue;
    auto file = directory / "sample.json";
    writeFile(file, "{\"ok\": true}");
    check(datafile::loadFile(file, fileValue, error), "loadFile valid: " + error);
    check(fileValue.at("ok").boolean(), "loadFile value");
    writeFile(file, "{\"ok\": }");
    check(!datafile::loadFile(file, fileValue, error), "loadFile invalid accepted");
    check(error.find("sample.json") != std::string::npos && error.find("line 1") != std::string::npos,
          "loadFile error names file and position: " + error);
    check(!datafile::loadFile(directory / "missing.json", fileValue, error), "missing file accepted");
}

void storySave(const std::filesystem::path& directory) {
    auto file = directory / "story-v4.dat";
    auto legacy = directory / "story-v3.dat";
    std::string error;
    GameSession session = newSession();
    session.story.flags = { { "met_warden", 1 }, { "chose_ledger", -2 } };
    session.story.choices = { { "warden_intro", "accept", "The warden gives you the ledger key." } };
    session.story.knownRecipes = { "iron_sword_recipe", "healing_draught_recipe" };
    session.story.factions = { { "ironward", 25 }, { "lantern", -10 } };
    for (const auto& entry : session.story.factions)
        check(guildDatabase().find(entry.factionId) != nullptr, "test faction exists");

    check(saves::save(session, file, error), "Cannot save story state: " + error);
    auto loaded = saves::load(file, error);
    check(loaded.has_value(), "Cannot load story state: " + error);
    check(loaded->story.flags.size() == 2 && loaded->story.flags[0].id == "met_warden" &&
          loaded->story.flags[1].value == -2, "Story flags lost");
    check(loaded->story.choices.size() == 1 && loaded->story.choices[0].choiceId == "accept" &&
          loaded->story.choices[0].result == "The warden gives you the ledger key.", "Choice log lost");
    check(loaded->story.knownRecipes == session.story.knownRecipes, "Known recipes lost");
    check(loaded->story.standing("ironward") == 25 && loaded->story.standing("lantern") == -10,
          "Faction standing lost");
    check(loaded->story.flag("met_warden") && loaded->story.flag("met_warden")->value == 1, "Flag lookup");
    for (const auto& item : loaded->inventory.inventory)
        check(!item.id.empty(), "Item id missing after load");
    check(loaded->inventory.inventory.front().id == "leather_armor", "Item id resolved from catalog");

    // Version 3 saves carry no story section and no item ids. Loading must fill
    // ids from the catalog and start with empty story state.
    check(saves::saveWithFormat(session, legacy, 3, error), "Cannot write version 3 save: " + error);
    auto old = saves::load(legacy, error);
    check(old.has_value(), "Version 3 migration failed: " + error);
    check(old->story.flags.empty() && old->story.choices.empty() &&
          old->story.knownRecipes.empty() && old->story.factions.empty(), "Migration invented story state");
    for (const auto& item : old->inventory.inventory)
        check(!item.id.empty() && old->inventory.indexOfItem(item.type, item.id) ==
              static_cast<int>(item.dbIndex), "Migration did not backfill item ids");
    check(saves::save(*old, file, error), "Migrated save cannot be written: " + error);
    loaded = saves::load(file, error);
    check(loaded.has_value() && loaded->inventory.inventory.front().id == "leather_armor",
          "Migrated save did not round-trip");

    // Version 1 and 2 saves still load with the current field layout.
    for (int version : { 1, 2 }) {
        check(saves::saveWithFormat(session, legacy, version, error), "Cannot write legacy save: " + error);
        auto oldSave = saves::load(legacy, error);
        check(oldSave.has_value(), "Version " + std::to_string(version) + " migration failed: " + error);
        check(oldSave->story.flags.empty(), "Legacy version invented story flags");
        for (const auto& item : oldSave->inventory.inventory)
            check(!item.id.empty(), "Legacy version did not backfill item ids");
    }

    // Unknown or mismatched item ids are hard errors.
    check(saves::save(session, file, error), "Cannot save for corruption test");
    const auto data = readFile(file);
    auto copy = directory / "story-corrupt.dat";
    auto corrupt = data;
    auto itemSection = corrupt.find("\"ITEM_IDS\"\n");
    check(itemSection != std::string::npos, "Item id section missing");
    auto id = corrupt.find("\"leather_armor\"", itemSection);
    check(id != std::string::npos, "Item id fixture missing");
    corrupt.replace(id, std::string("\"leather_armor\"").size(), "\"unknown_item\"");
    writeFile(copy, corrupt);
    check(!saves::load(copy, error), "Unknown item id accepted");

    corrupt = data;
    id = corrupt.find("\"leather_armor\"", itemSection);
    corrupt.replace(id, std::string("\"leather_armor\"").size(), "\"iron_armor\"");
    writeFile(copy, corrupt);
    check(!saves::load(copy, error), "Mismatched item id accepted");

    // Story state is validated on save, not only on load.
    GameSession bad = session;
    bad.story.factions = { { "ironward", 500 } };
    check(!saves::save(bad, copy, error), "Out-of-range standing accepted");
    bad.story.factions = { { "not_a_guild", 0 } };
    check(!saves::save(bad, copy, error), "Unknown faction accepted");
    bad = session;
    bad.story.flags = { { "dup", 1 }, { "dup", 2 } };
    check(!saves::save(bad, copy, error), "Duplicate flag accepted");
    bad = session;
    bad.story.choices = { { "warden_intro", "accept", "one" }, { "warden_intro", "accept", "two" } };
    check(!saves::save(bad, copy, error), "Duplicate choice accepted");
    bad = session;
    bad.story.knownRecipes = { "iron_sword_recipe", "iron_sword_recipe" };
    check(!saves::save(bad, copy, error), "Duplicate recipe accepted");
    bad = session;
    bad.story.flags = { { "Bad Id", 1 } };
    check(!saves::save(bad, copy, error), "Invalid flag id accepted");
    bad = session;
    bad.inventory.inventory.front().dbIndex = 999999;
    check(!saves::save(bad, copy, error), "Invalid item index accepted");
}

} // namespace

int main(int argc, char** argv) {
    try {
        check(argc >= 2, "Pass test group");
        std::string group = argv[1];
        std::filesystem::path directory = argc >= 3 ? std::filesystem::path(argv[2]) : std::filesystem::temp_directory_path();
        std::filesystem::create_directories(directory);
        if (group == "ids") idLayer();
        else if (group == "json") jsonReader(directory);
        else if (group == "save") storySave(directory);
        else throw std::runtime_error("Unknown test group");
        std::cout << "Passed " << group << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
