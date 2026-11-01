#include "material.hpp"

#include <cmath>
#include <set>
#include <stdexcept>

#include "core/ids.hpp"
#include "core/paths.hpp"
#include "entities/enemy.hpp"
#include "gameplay/datafile.hpp"
#include "world/quest_system.hpp"

namespace {

std::string fieldError(const std::string& where, const std::string& message) {
    return where + ": " + message;
}

} // namespace

bool MaterialDatabase::load(std::string& error) {
    return load(paths::dataFile("crafting/materials.json"), error);
}

bool MaterialDatabase::load(const std::filesystem::path& path, std::string& error) {
    error.clear();
    materials.clear();
    loaded_ = false;
    error_.clear();

    datafile::Value root;
    if (!datafile::loadFile(path, root, error)) {
        error_ = error;
        return false;
    }

    const std::string file = path.filename().string();
    try {
        EnemyDatabase enemies;
        std::set<std::string> ids;
        const auto& entries = root.at("materials").array();
        if (entries.empty()) throw std::runtime_error("materials: catalog is empty");
        materials.reserve(entries.size());
        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            const std::string where = "materials[" + std::to_string(i) + "]";
            MaterialDefinition material;
            material.id = entry.at("id").string();
            if (!ids::valid(material.id))
                throw std::runtime_error(fieldError(where + ".id", "invalid snake_case id"));
            if (!ids.insert(material.id).second)
                throw std::runtime_error(fieldError(where + ".id", "duplicate id"));
            material.name = entry.at("name").string();
            if (material.name.empty()) throw std::runtime_error(fieldError(where + ".name", "empty name"));
            material.description = entry.at("description").string();
            material.source = entry.has("source") ? entry.at("source").string() : "";
            material.weight = entry.at("weight").number();
            if (!(material.weight > 0.0) || material.weight > 1000.0)
                throw std::runtime_error(fieldError(where + ".weight", "must be between 0 and 1000"));
            material.sellValue = entry.at("sellValue").integer();
            if (material.sellValue < 0 || material.sellValue > 100000000)
                throw std::runtime_error(fieldError(where + ".sellValue", "out of range"));
            material.maxStack = entry.has("maxStack") ? entry.at("maxStack").integer() : 999;
            if (material.maxStack < 1 || material.maxStack > 1000000)
                throw std::runtime_error(fieldError(where + ".maxStack", "must be between 1 and 1000000"));

            if (entry.has("drops")) {
                const auto& drops = entry.at("drops").array();
                for (size_t d = 0; d < drops.size(); ++d) {
                    const std::string dropWhere = where + ".drops[" + std::to_string(d) + "]";
                    MaterialDrop drop;
                    drop.enemyId = drops[d].at("enemy").string();
                    if (enemies.find(drop.enemyId) == nullptr)
                        throw std::runtime_error(fieldError(dropWhere + ".enemy", "unknown enemy id"));
                    drop.chance = static_cast<float>(drops[d].at("chance").number());
                    if (!(drop.chance > 0.0f) || drop.chance > 1.0f)
                        throw std::runtime_error(fieldError(dropWhere + ".chance", "must be between 0 and 1"));
                    drop.minQuantity = drops[d].at("minQuantity").integer();
                    drop.maxQuantity = drops[d].at("maxQuantity").integer();
                    if (drop.minQuantity < 1 || drop.maxQuantity < drop.minQuantity ||
                        drop.maxQuantity > 1000)
                        throw std::runtime_error(fieldError(dropWhere, "invalid quantity range"));
                    drop.materialId = material.id;
                    material.drops.push_back(drop);
                }
            }
            if (entry.has("questRewards")) {
                std::set<std::string> quests;
                for (const auto& reward : entry.at("questRewards").array()) {
                    const auto id = reward.at("quest").string();
                    int quantity = reward.at("quantity").integer();
                    if (!findQuest(id) || !quests.insert(id).second || quantity < 1 || quantity > material.maxStack)
                        throw std::runtime_error(fieldError(where + ".questRewards", "invalid quest reward"));
                    material.questRewards.emplace_back(id, quantity);
                }
            }
            materials.push_back(std::move(material));
        }
    } catch (const std::exception& e) {
        error = file + ": " + std::string(e.what());
        error_ = error;
        materials.clear();
        return false;
    }

    loaded_ = true;
    return true;
}

const MaterialDefinition* MaterialDatabase::find(const std::string& id) const {
    for (const auto& material : materials)
        if (material.id == id) return &material;
    return nullptr;
}

int MaterialDatabase::indexOf(const std::string& id) const {
    for (size_t i = 0; i < materials.size(); ++i)
        if (materials[i].id == id) return static_cast<int>(i);
    return -1;
}

const MaterialDatabase& materialDatabase() {
    static MaterialDatabase database = [] {
        MaterialDatabase loaded;
        std::string error;
        loaded.load(error);
        return loaded;
    }();
    return database;
}

std::vector<MaterialDrop> dropsFor(const std::string& enemyId) {
    std::vector<MaterialDrop> result;
    for (const auto& material : materialDatabase().getMaterials())
        for (const auto& drop : material.drops)
            if (drop.enemyId == enemyId) result.push_back(drop);
    return result;
}

std::vector<MaterialGain> rollDrops(const std::string& enemyId, float roll) {
    std::vector<MaterialGain> result;
    if (!(roll >= 0.0f) || roll >= 1.0f) return result;
    for (const auto& drop : dropsFor(enemyId)) {
        if (roll >= drop.chance) continue;
        int span = drop.maxQuantity - drop.minQuantity + 1;
        int quantity = drop.minQuantity + static_cast<int>((roll / drop.chance) * static_cast<float>(span));
        if (quantity > drop.maxQuantity) quantity = drop.maxQuantity;
        result.push_back({drop.materialId, quantity});
    }
    return result;
}

std::vector<MaterialGain> rollDrops(const std::string& enemyId, std::mt19937& random) {
    std::vector<MaterialGain> result;
    for (const auto& drop : dropsFor(enemyId)) {
        if (std::uniform_real_distribution<float>(0.0f, 1.0f)(random) >= drop.chance) continue;
        result.push_back({drop.materialId, std::uniform_int_distribution<int>(drop.minQuantity, drop.maxQuantity)(random)});
    }
    return result;
}
