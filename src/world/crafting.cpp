#include "crafting.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "app/display.hpp"
#include "catalogs/content_ids.hpp"
#include "catalogs/content_paths.hpp"
#include "core/ui.hpp"
#include "catalogs/guild.hpp"
#include "gameplay/datafile.hpp"
#include "gameplay/guild.hpp"
#include "gameplay/inventory.hpp"
#include "world/quest_system.hpp"

namespace {

std::string fieldError(const std::string& where, const std::string& message) {
    return where + ": " + message;
}

std::string inputList(const PlayerInventory& inventory, const Recipe& recipe) {
    std::string result;
    for (const auto& input : recipe.inputs) {
        PlayerInventory::ItemType type;
        size_t index = 0;
        std::string name = input.itemId;
        if (inventory.resolveItemId(input.itemId, type, index)) name = inventory.itemName(type, index);
        if (!result.empty()) result += ", ";
        result += std::to_string(input.quantity) + " " + name;
    }
    return result;
}

} // namespace

bool CraftingDatabase::load(std::string& error) {
    return load(paths::dataFile("crafting/recipes.json"), error);
}

bool CraftingDatabase::load(const std::filesystem::path& path, std::string& error) {
    error.clear();
    recipes.clear();
    loaded_ = false;
    error_.clear();

    datafile::Value root;
    if (!datafile::loadFile(path, root, error)) {
        error_ = error;
        return false;
    }

    const std::string file = path.filename().string();
    try {
        PlayerInventory catalog;
        std::set<std::string> usedIds;
        const auto stationIds = craftStations();
        std::set<std::string> stations(stationIds.begin(), stationIds.end());
        const auto& entries = root.at("recipes").array();
        if (entries.empty()) throw std::runtime_error("recipes: catalog is empty");
        recipes.reserve(entries.size());
        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            const std::string where = "recipes[" + std::to_string(i) + "]";
            Recipe recipe;
            recipe.id = entry.at("id").string();
            if (!ids::valid(recipe.id))
                throw std::runtime_error(fieldError(where + ".id", "invalid snake_case id"));
            if (!usedIds.insert(recipe.id).second)
                throw std::runtime_error(fieldError(where + ".id", "duplicate id"));
            recipe.name = entry.at("name").string();
            if (recipe.name.empty()) throw std::runtime_error(fieldError(where + ".name", "empty name"));
            recipe.station = entry.at("station").string();
            if (stations.count(recipe.station) == 0)
                throw std::runtime_error(fieldError(where + ".station", "unknown station"));

            std::set<std::string> inputIds;
            const auto& inputs = entry.at("inputs").array();
            if (inputs.empty()) throw std::runtime_error(fieldError(where + ".inputs", "at least one input is required"));
            for (size_t k = 0; k < inputs.size(); ++k) {
                const std::string inputWhere = where + ".inputs[" + std::to_string(k) + "]";
                RecipeInput input;
                input.itemId = inputs[k].at("item").string();
                PlayerInventory::ItemType type;
                size_t index = 0;
                if (!catalog.resolveItemId(input.itemId, type, index))
                    throw std::runtime_error(fieldError(inputWhere + ".item", "unknown item id"));
                if (!inputIds.insert(input.itemId).second)
                    throw std::runtime_error(fieldError(inputWhere + ".item", "duplicate input id"));
                input.quantity = inputs[k].at("quantity").integer();
                if (input.quantity < 1 || input.quantity > 1000)
                    throw std::runtime_error(fieldError(inputWhere + ".quantity", "must be between 1 and 1000"));
                recipe.inputs.push_back(std::move(input));
            }

            const auto& output = entry.at("output");
            recipe.outputId = output.at("item").string();
            PlayerInventory::ItemType outputType;
            size_t outputIndex = 0;
            if (!catalog.resolveItemId(recipe.outputId, outputType, outputIndex))
                throw std::runtime_error(fieldError(where + ".output.item", "unknown item id"));
            recipe.outputQuantity = output.at("quantity").integer();
            if (recipe.outputQuantity < 1 || recipe.outputQuantity > 1000)
                throw std::runtime_error(fieldError(where + ".output.quantity", "must be between 1 and 1000"));

            const auto& unlock = entry.at("unlock");
            const std::string kind = unlock.at("type").string();
            if (kind == "always") {
                recipe.unlock = Recipe::Unlock::Always;
            } else {
                recipe.unlockId = unlock.at("id").string();
                if (kind == "flag") {
                    recipe.unlock = Recipe::Unlock::Flag;
                    if (!ids::valid(recipe.unlockId))
                        throw std::runtime_error(fieldError(where + ".unlock.id", "invalid flag id"));
                } else if (kind == "guild") {
                    recipe.unlock = Recipe::Unlock::Guild;
                    if (guildDatabase().find(recipe.unlockId) == nullptr)
                        throw std::runtime_error(fieldError(where + ".unlock.id", "unknown guild id"));
                } else if (kind == "quest") {
                    recipe.unlock = Recipe::Unlock::Quest;
                    if (findQuest(recipe.unlockId) == nullptr)
                        throw std::runtime_error(fieldError(where + ".unlock.id", "unknown quest id"));
                } else {
                    throw std::runtime_error(fieldError(where + ".unlock.type", "expected always, flag, guild, or quest"));
                }
            }
            recipes.push_back(std::move(recipe));
        }
    } catch (const std::exception& e) {
        error = file + ": " + e.what();
        error_ = error;
        recipes.clear();
        return false;
    }

    loaded_ = true;
    return true;
}

const Recipe* CraftingDatabase::find(const std::string& id) const {
    for (const auto& recipe : recipes)
        if (recipe.id == id) return &recipe;
    return nullptr;
}

std::vector<const Recipe*> CraftingDatabase::atStation(const std::string& station) const {
    std::vector<const Recipe*> result;
    for (const auto& recipe : recipes)
        if (recipe.station == station) result.push_back(&recipe);
    return result;
}

const CraftingDatabase& craftingDatabase() {
    static CraftingDatabase database = [] {
        CraftingDatabase loaded;
        std::string error;
        loaded.load(error);
        return loaded;
    }();
    return database;
}

std::vector<std::string> craftStations() {
    return { "forge", "alchemy", "kitchen" };
}

std::string craftStationName(const std::string& station) {
    if (station == "forge") return "Forge";
    if (station == "alchemy") return "Alchemy Bench";
    if (station == "kitchen") return "Kitchen";
    return station;
}

std::string recipeLockReason(const GameSession& session, const Recipe& recipe) {
    if (std::find(session.story.knownRecipes.begin(), session.story.knownRecipes.end(), recipe.id) !=
        session.story.knownRecipes.end()) return {};
    switch (recipe.unlock) {
        case Recipe::Unlock::Always:
            return {};
        case Recipe::Unlock::Flag: {
            const auto* flag = session.story.flag(recipe.unlockId);
            if (flag && flag->value != 0) return {};
            return "requires the story flag \"" + recipe.unlockId + "\"";
        }
        case Recipe::Unlock::Guild: {
            const auto* progress = guildProgress(session.guilds, recipe.unlockId);
            if (progress && progress->joined) return {};
            const auto* guild = guildDatabase().find(recipe.unlockId);
            return "join " + (guild ? guild->name : recipe.unlockId);
        }
        case Recipe::Unlock::Quest: {
            if (questProgress(session, recipe.unlockId).completions > 0) return {};
            const auto* quest = findQuest(recipe.unlockId);
            return "complete " + (quest ? quest->title : recipe.unlockId);
        }
    }
    return "locked";
}

bool recipeUnlocked(const GameSession& session, const Recipe& recipe) {
    return recipeLockReason(session, recipe).empty();
}

void discoverRecipes(GameSession& session) {
    for (const auto& recipe : craftingDatabase().getRecipes()) {
        if (recipeUnlocked(session, recipe) &&
            std::find(session.story.knownRecipes.begin(), session.story.knownRecipes.end(), recipe.id) == session.story.knownRecipes.end())
            session.story.knownRecipes.push_back(recipe.id);
    }
}

bool craft(GameSession& session, const Recipe& recipe, std::string& error) {
    error.clear();
    if (session.hero.currentLocationType != PeacefulCity || session.hero.stats.hitpoints <= 0) {
        error = "Crafting requires a city station and a living crafter.";
        return false;
    }
    if (recipe.inputs.empty() || recipe.outputQuantity <= 0) {
        error = "Invalid recipe quantities.";
        return false;
    }
    const std::string lock = recipeLockReason(session, recipe);
    if (!lock.empty()) {
        error = "Recipe locked: " + lock + ".";
        return false;
    }

    auto result = session.inventory;
    for (const auto& input : recipe.inputs) {
        PlayerInventory::ItemType type;
        size_t index = 0;
        if (!session.inventory.resolveItemId(input.itemId, type, index)) {
            error = "Unknown ingredient: " + input.itemId + ".";
            return false;
        }
        const int owned = result.countItem(type, index);
        if (input.quantity <= 0 || !result.removeItem(type, index, input.quantity)) {
            error = "Not enough " + session.inventory.itemName(type, index) + ": need " +
                    std::to_string(input.quantity) + ", have " + std::to_string(owned) + ".";
            return false;
        }
    }

    PlayerInventory::ItemType outputType;
    size_t outputIndex = 0;
    if (!session.inventory.resolveItemId(recipe.outputId, outputType, outputIndex)) {
        error = "Unknown result: " + recipe.outputId + ".";
        return false;
    }
    if (!result.tryAddItem(outputType, outputIndex, recipe.outputQuantity)) {
        error = "Your backpack is full. Crafting would exceed its weight limit.";
        return false;
    }

    session.inventory = std::move(result);
    if (std::find(session.story.knownRecipes.begin(), session.story.knownRecipes.end(), recipe.id) == session.story.knownRecipes.end())
        session.story.knownRecipes.push_back(recipe.id);
    return true;
}

void openCrafting(GameSession& session, const std::string& station) {
    if (session.hero.currentLocationType != PeacefulCity) return;
    discoverRecipes(session);
    std::string current = station;
    if (current.empty()) {
        std::vector<std::string> lines;
        const auto stations = craftStations();
        for (size_t i = 0; i < stations.size(); ++i)
            lines.push_back(std::to_string(i + 1) + ". " + craftStationName(stations[i]));
        lines.push_back("0. Back");
        clearScreen();
        displayBorderedMenu(lines, "Choose a station: ");
        const int choice = getNumberInput(0, static_cast<int>(stations.size()));
        if (choice == 0) return;
        current = stations[static_cast<size_t>(choice - 1)];
    }

    while (true) {
        clearScreen();
        const auto recipes = craftingDatabase().atStation(current);
        std::vector<std::string> lines;
        lines.push_back("Crafting: " + craftStationName(current));
        lines.push_back("Backpack: " + std::to_string(static_cast<int>(session.inventory.totalWeight())) +
                        "/" + std::to_string(static_cast<int>(PlayerInventory::capacityWeight)) + " weight" +
                        (session.inventory.encumbered() ? " [encumbered]" : ""));
        for (size_t i = 0; i < recipes.size(); ++i) {
            const auto& recipe = *recipes[i];
            const std::string lock = recipeLockReason(session, recipe);
            lines.push_back(std::to_string(i + 1) + ". " + recipe.name +
                            " (" + inputList(session.inventory, recipe) + ")" +
                            (lock.empty() ? "" : " [Locked: " + lock + "]"));
        }
        lines.push_back("0. Back");
        displayBorderedMenu(lines, "Choose a recipe: ");
        const int choice = getNumberInput(0, static_cast<int>(recipes.size()));
        if (choice == 0) return;
        const auto& recipe = *recipes[static_cast<size_t>(choice - 1)];
        std::string error;
        if (craft(session, recipe, error))
            display::message("Crafted " + recipe.name + " x" + std::to_string(recipe.outputQuantity) + ".");
        else
            display::message(error);
    }
}
