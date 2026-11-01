#pragma once

#include <filesystem>
#include <random>
#include <string>
#include <vector>

// Crafting materials. Unlike the older catalogs these live in
// `data/materials.json`, so they can grow without recompiling.
struct MaterialDrop {
    std::string enemyId;
    float chance = 0.0f;
    int minQuantity = 1;
    int maxQuantity = 1;
    std::string materialId; // filled by the loader, not read from the file
};

// One rolled drop, before it is added to a backpack.
struct MaterialGain {
    std::string materialId;
    int quantity = 1;
};

struct MaterialDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string source;
    double weight = 0.0;
    int sellValue = 0; // copper
    int maxStack = 999;
    std::vector<MaterialDrop> drops;
    std::vector<std::pair<std::string, int>> questRewards;
};

class MaterialDatabase {
public:
    // Loads `data/materials.json`. Syntax errors name a file, line, and column;
    // schema errors name a file and field. A failed load leaves the catalog empty.
    bool load(std::string& error);
    bool load(const std::filesystem::path& path, std::string& error);

    bool loaded() const { return loaded_; }
    const std::string& error() const { return error_; }
    const std::vector<MaterialDefinition>& getMaterials() const { return materials; }
    const MaterialDefinition* find(const std::string& id) const;
    int indexOf(const std::string& id) const;

private:
    std::vector<MaterialDefinition> materials;
    bool loaded_ = false;
    std::string error_;
};

// Process-wide catalog, loaded on first use. Empty when the file is missing.
const MaterialDatabase& materialDatabase();

// Every drop rule attached to `enemyId`.
std::vector<MaterialDrop> dropsFor(const std::string& enemyId);

// Applies one roll in [0, 1) to every drop rule of the enemy. Test code passes
// 0.0f to collect all drops deterministically. Quantity scales with the roll so
// a high roll yields more than a low one.
std::vector<MaterialGain> rollDrops(const std::string& enemyId, float roll);
std::vector<MaterialGain> rollDrops(const std::string& enemyId, std::mt19937& random);
