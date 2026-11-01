#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "app/session.hpp"
#include "combat/combat.hpp"
#include "core/paths.hpp"
#include "catalogs/class.hpp"
#include "catalogs/race.hpp"
#include "gameplay/guild.hpp"
#include "gameplay/save.hpp"
#include "world/crafting.hpp"
#include "world/establishments.hpp"

namespace {
using Type = PlayerInventory::ItemType;
void check(bool ok, const std::string& message) { if (!ok) throw std::runtime_error(message); }
GameSession game() {
    PlayerClassCollection classes;
    PlayerRaceDatabase races;
    GameSession s(Player("Crafter", {races.templates[0].name, {races.templates[0].lore.description}}, classes.getClassTemplate("Warrior")));
    s.hero.currentLocation = "Attania";
    s.hero.currentLocationType = PeacefulCity;
    return s;
}
size_t material(const std::string& id) {
    int index = materialDatabase().indexOf(id);
    check(index >= 0, "Missing material " + id);
    return static_cast<size_t>(index);
}
void supply(GameSession& s) {
    s.inventory.addItem(Type::Material, material("iron_ore"), 3);
    s.inventory.addItem(Type::Material, material("rough_leather"));
}
std::string contents(const std::filesystem::path& path) {
    std::ifstream in(path);
    return {std::istreambuf_iterator<char>(in), {}};
}
void write(const std::filesystem::path& path, const std::string& text) { std::ofstream(path) << text; }
struct Input {
    std::istringstream in;
    std::ostringstream out;
    std::streambuf* oldIn;
    std::streambuf* oldOut;
    explicit Input(const std::string& text) : in(text), oldIn(std::cin.rdbuf(in.rdbuf())), oldOut(std::cout.rdbuf(out.rdbuf())) {}
    ~Input() { std::cin.rdbuf(oldIn); std::cout.rdbuf(oldOut); std::cin.clear(); }
};

void catalogs(const std::filesystem::path& directory) {
    check(materialDatabase().loaded(), materialDatabase().error());
    check(craftingDatabase().loaded(), craftingDatabase().error());
    check(craftingDatabase().getRecipes().size() >= 9, "Missing recipes");
    const auto good = contents(paths::dataFile("crafting/recipes.json"));
    const auto file = directory / "invalid-recipes.json";
    for (const auto& replacement : {std::pair<std::string, std::string>{"iron_ore", "missing_item"},
             {"forge", "missing_station"}, {"ironward", "missing_guild"}, {"side_waystone", "missing_quest"}}) {
        auto text = good;
        text.replace(text.find(replacement.first), replacement.first.size(), replacement.second);
        write(file, text);
        CraftingDatabase db;
        std::string error;
        check(!db.load(file, error) && error.find("invalid-recipes.json") != std::string::npos, "Accepted bad recipe reference");
        check(db.getRecipes().empty(), "Failed load retained partial recipes");
    }
    write(file, "{\"recipes\":[{\"id\":\"missing_fields\"}]}");
    CraftingDatabase db;
    std::string error;
    check(!db.load(file, error) && error.find("name") != std::string::npos, "Missing field not diagnosed");
    auto text = good;
    text.replace(text.find("iron_armor_recipe"), 17, "iron_sword_recipe");
    write(file, text);
    check(!db.load(file, error) && error.find("duplicate") != std::string::npos, "Duplicate recipe accepted");
    auto materials = contents(paths::dataFile("crafting/materials.json"));
    materials.replace(materials.find("cave_troll"), 10, "missing_enemy");
    write(file, materials);
    MaterialDatabase md;
    check(!md.load(file, error), "Unknown drop enemy accepted");
}

void transactions() {
    auto s = game();
    const auto& recipe = *craftingDatabase().find("iron_sword_recipe");
    std::string error;
    check(!craft(s, recipe, error), "Crafted without materials");
    supply(s);
    // Ingredients occupy 6.8 weight; output occupies 4.0. A full backpack can
    // still craft when the result is lighter than the consumed ingredients.
    s.inventory.addItem(Type::Potion, 0, 86);
    check(craft(s, recipe, error), "Net-weight craft failed: " + error);
    check(s.inventory.countItem(Type::Material, material("iron_ore")) == 0, "Inputs not consumed");
    check(s.inventory.countItem(Type::Equipment, s.inventory.equipmentDB.indexOf("iron_sword")) == 1, "Output missing");
    check(s.story.knownRecipes.size() == 1, "Recipe not recorded");
    auto before = s.inventory.totalWeight();
    check(!craft(s, recipe, error) && s.inventory.totalWeight() == before, "Failed craft mutated inventory");

    s = game();
    supply(s);
    s.inventory.addItem(Type::Potion, 0, 86);
    auto heavy = recipe;
    heavy.outputQuantity = 2;
    check(!craft(s, heavy, error), "Overweight craft succeeded");
    check(s.inventory.countItem(Type::Material, material("iron_ore")) == 3, "Failed craft consumed ore");
    s.hero.currentLocationType = Terrain;
    check(!craft(s, recipe, error), "Crafted outside a city");
    s = game();
    const auto& armor = *craftingDatabase().find("iron_armor_recipe");
    check(!recipeUnlocked(s, armor), "Guild recipe starts unlocked");
    supply(s);
    check(!craft(s, armor, error) && s.inventory.countItem(Type::Material, material("iron_ore")) == 3, "Locked craft consumed ingredients");
    check(joinGuild(s, "ironward") && recipeUnlocked(s, armor), "Guild recipe did not unlock");
    const auto& steel = *craftingDatabase().find("steel_sword_recipe");
    check(!recipeUnlocked(s, steel), "Flag recipe starts unlocked");
    s.story.flags.push_back({"smith_mastery", 1});
    check(recipeUnlocked(s, steel), "Flag recipe did not unlock");
    const auto& potion = *craftingDatabase().find("large_health_potion_recipe");
    check(!recipeUnlocked(s, potion), "Quest recipe starts unlocked");
    s.quests.jobs.push_back({"side_waystone", false, false, 1, "ironward"});
    check(recipeUnlocked(s, potion), "Quest recipe did not unlock");
    discoverRecipes(s);
    check(std::find(s.story.knownRecipes.begin(), s.story.knownRecipes.end(), steel.id) != s.story.knownRecipes.end(), "Discovery missing");
}

void backpack() {
    auto s = game();
    s.hero.stats.attack = 100;
    s.hero.stats.magicAttack = 50;
    s.hero.stats.dodgeRate = 0.20f;
    PlayerCombatant combatant(s.hero);
    combatant.setInventory(s.inventory);
    check(s.inventory.tryAddItem(Type::Potion, 0, 75), "Could not reach soft threshold");
    check(!s.inventory.encumbered() && combatant.getPhysicalAttack() == 100, "Penalty at exactly soft threshold");
    check(s.inventory.tryAddItem(Type::Potion, 0), "Could not cross soft threshold");
    check(s.inventory.encumbered() && combatant.getPhysicalAttack() == 90 && combatant.getMagicAttack() == 45, "Attack penalty missing");
    check(std::abs(combatant.getDodgeRate() - 0.10f) < 0.0001f, "Dodge penalty missing");
    check(s.hero.stats.attack == 100, "Penalty mutated saved stats");
    check(s.inventory.tryAddItem(Type::Potion, 0, 24), "Could not fill backpack");
    check(!s.inventory.tryAddItem(Type::Potion, 0) && s.inventory.countItem(Type::Potion, 0) == 100, "Hard cap failed");
    check(!s.inventory.tryAddItem(Type::Material, material("wild_herb")), "Material bypassed cap");
    check(!s.inventory.canAdd(Type::Potion, 9999) && !s.inventory.canAdd(Type::Potion, 0, -1), "Invalid pickup accepted");
    std::string error;
    check(sellItem(s.hero, s.inventory, 0, 25, error), "Cannot sell burden");
    check(!s.inventory.encumbered() && combatant.getPhysicalAttack() == 100, "Penalty not removed");
    check(s.hero.economy.gold == 1 && s.hero.economy.silver == 50, "Unexpected sale price");
    s.inventory.inventory.clear();
    check(!s.inventory.tryAddItem(Type::Material, material("wild_herb"), 1000), "Material stack cap bypassed");
}

void rewards(const std::filesystem::path& directory) {
    auto s = game();
    check(!rollDrops("sludge", 0.0f).empty(), "Sludge drops missing");
    check(!rollDrops("direwolf", 0.0f).empty(), "Wolf drops missing");
    check(!rollDrops("bonewalker", 0.0f).empty(), "Bonewalker drops missing");
    check(rollDrops("sludge", 0.99f).empty(), "Failed roll awarded loot");
    std::mt19937 random(42);
    bool sawMaximum = false;
    for (int n = 0; n < 200; ++n)
        for (const auto& gain : rollDrops("sludge", random)) {
            check(gain.quantity >= 1 && gain.quantity <= 2, "Bad drop quantity");
            if (gain.quantity == 2) sawMaximum = true;
        }
    check(sawMaximum, "Maximum drop quantity unreachable");
    check(acceptQuest(s, "commission_supplies"), "Cannot accept supplies");
    s.hero.currentLocation = "Barren Plains";
    check(completeQuestObjective(s, "commission_supplies", true), "Cannot finish supplies");
    s.hero.currentLocation = "Attania";
    s.inventory.addItem(Type::Potion, 0, 100);
    check(!claimQuestReward(s, "commission_supplies"), "Full pack accepted reward");
    check(questProgress(s, "commission_supplies").objectiveComplete, "Full pack lost reward");
    s.inventory.inventory.clear();
    { Input io("\n"); check(claimQuestReward(s, "commission_supplies"), "Cannot claim materials"); }
    check(!claimQuestReward(s, "commission_supplies"), "Reward replayed");
    std::string error;
    check(saves::save(s, directory / "workflow-start.dat", error), "Cannot save workflow fixture");
    check(craft(s, *craftingDatabase().find("iron_sword_recipe"), error), "Quest materials cannot craft");
    s.inventory.addItem(Type::Material, material("bone_shard"), 2);
    const auto path = directory / "craft-save.dat";
    check(saves::save(s, path, error), "Save failed: " + error);
    auto loaded = saves::load(path, error);
    check(loaded.has_value(), "Load failed: " + error);
    check(loaded->inventory.countItem(Type::Material, material("bone_shard")) == 2, "Materials lost");
    check(loaded->story.knownRecipes == s.story.knownRecipes, "Recipes lost");
    check(loaded->inventory.countItem(Type::Equipment, loaded->inventory.equipmentDB.indexOf("iron_sword")) == 1, "Crafted item lost");
    s.inventory.equipped.weaponIndex = s.inventory.equipmentDB.indexOf("iron_sword");
    check(!sellItem(s.hero, s.inventory, 0, 1, error), "Sold last equipped copy");
    s.inventory.equipped.weaponIndex = -1;
    check(sellItem(s.hero, s.inventory, 0, 1, error), "Cannot sell crafted sword");
}

void menus() {
    auto s = game();
    supply(s);
    { Input io("1\n1\n\n0\n"); openCrafting(s); check(io.out.str().find("Crafted Iron Sword") != std::string::npos, "Craft menu failed"); }
    { Input io("3\n1\n1\n4\n"); Store(s.inventory).openStore(s.hero); check(io.out.str().find("Sold Iron Sword") != std::string::npos, "Sell menu failed"); }
    check(s.inventory.inventory.empty(), "Menu sale retained item");
    s.inventory.addItem(Type::Potion, 0, 100);
    auto money = s.hero.economy;
    { Input io("1\n1\n\n4\n"); Store(s.inventory).openStore(s.hero); check(io.out.str().find("Purchase cancelled") != std::string::npos, "Full-pack purchase not blocked"); }
    check(s.hero.economy.silver == money.silver && s.hero.economy.gold == money.gold, "Blocked purchase charged money");
}
}

int main(int argc, char** argv) {
    try {
        check(argc == 3, "Pass group and scratch directory");
        const auto directory = std::filesystem::path(argv[2]);
        std::filesystem::create_directories(directory);
        std::string group = argv[1];
        if (group == "catalogs") catalogs(directory);
        else if (group == "transactions") transactions();
        else if (group == "backpack") backpack();
        else if (group == "rewards") rewards(directory);
        else if (group == "menus") menus();
        else throw std::runtime_error("Unknown group");
        std::cout << "Passed " << group << '\n';
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
