#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct GameSession;

// One ingredient of a recipe, referenced by stable item id. Ids resolve across
// materials, equipment, potions, and food, so a recipe can consume any of them.
struct RecipeInput {
    std::string itemId;
    int quantity = 1;
};

struct Recipe {
    std::string id;
    std::string name;
    std::string station;
    // Station fee in copper, charged once per craft.
    int feeCopper = 0;
    std::vector<RecipeInput> inputs;
    std::string outputId;
    int outputQuantity = 1;
    enum class Unlock { Always, Flag, Guild, Quest };
    Unlock unlock = Unlock::Always;
    std::string unlockId;
};

// Recipes authored in `data/recipes.json`. The catalog is read once and then
// shared by the menu and the tests.
class CraftingDatabase {
public:
    // Loads the default `data/recipes.json`.
    bool load(std::string& error);
    // Loads a specific file. Used by tests and by the export step later.
    bool load(const std::filesystem::path& path, std::string& error);

    bool loaded() const { return loaded_; }
    const std::string& error() const { return error_; }
    const std::vector<Recipe>& getRecipes() const { return recipes; }
    const Recipe* find(const std::string& id) const;
    std::vector<const Recipe*> atStation(const std::string& station) const;

private:
    std::vector<Recipe> recipes;
    bool loaded_ = false;
    std::string error_;
};

// Process-wide catalog, loaded on first use. Empty when the file is missing.
const CraftingDatabase& craftingDatabase();

// Station ids in display order. A station is available in every city and in a
// guild hall; the menu decides where to offer the entry point.
std::vector<std::string> craftStations();

// Human-readable station name, e.g. "forge" -> "Forge".
std::string craftStationName(const std::string& station);

// Empty when the recipe is available, otherwise why it is locked.
std::string recipeLockReason(const GameSession& session, const Recipe& recipe);
bool recipeUnlocked(const GameSession& session, const Recipe& recipe);
void discoverRecipes(GameSession& session);

// Consumes the inputs and adds the output. On failure returns false and writes
// a player-facing reason. No output is printed here.
bool craft(GameSession& session, const Recipe& recipe, std::string& error);

// Interactive station and recipe menu. `station` empty shows the station choice.
void openCrafting(GameSession& session, const std::string& station = "");
