#include "upgrades.hpp"

#include <algorithm>
#include <stdexcept>

#include "app/display.hpp"
#include "app/session.hpp"
#include "catalogs/content_paths.hpp"
#include "core/ui.hpp"
#include "gameplay/datafile.hpp"
#include "world/reputation.hpp"

namespace {

std::string fieldError(const std::string& where, const std::string& message) {
    return where + ": " + message;
}

long long toCopper(const PlayerInventory::InventoryItem& item,
                   const EquipmentandWeaponDatabase& database) {
    const auto& equipment = database.getEquipment()[item.dbIndex];
    return static_cast<long long>(equipment.pricePlatinum) * 1000000LL +
           static_cast<long long>(equipment.priceGold) * 10000LL +
           static_cast<long long>(equipment.priceSilver) * 100LL +
           equipment.priceCopper;
}

std::string copperLabel(long long copper) {
    CoinAmount price;
    price.platinum = static_cast<int>(copper / 1000000);
    copper %= 1000000;
    price.gold = static_cast<int>(copper / 10000);
    copper %= 10000;
    price.silver = static_cast<int>(copper / 100);
    price.copper = static_cast<int>(copper % 100);
    return priceLabel(price);
}

std::string itemLabel(const PlayerInventory& inventory,
                      const PlayerInventory::InventoryItem& item) {
    std::string label = inventory.itemName(item.type, item.dbIndex);
    if (item.upgradeLevel > 0) label += " +" + std::to_string(item.upgradeLevel);
    return label;
}

bool isEquipment(const PlayerInventory::InventoryItem& item) {
    return item.type == PlayerInventory::ItemType::Equipment;
}

} // namespace

EquipmentUpgradeDatabase::EquipmentUpgradeDatabase() {
    datafile::Value root;
    std::string error;
    const auto path = paths::dataFile("items/upgrades.json");
    if (!datafile::loadFile(path, root, error)) {
        error_ = error;
        return;
    }
    try {
        if (root.at("schemaVersion").integer() != 1)
            throw std::runtime_error("unsupported schemaVersion");
        PlayerInventory catalog;
        const auto& levels = root.at("levels").array();
        if (levels.empty()) throw std::runtime_error("levels: upgrade catalog is empty");
        for (size_t i = 0; i < levels.size(); ++i) {
            const std::string where = "levels[" + std::to_string(i) + "]";
            UpgradeRule rule;
            rule.level = levels[i].at("level").integer();
            if (rule.level != static_cast<int>(i) + 1)
                throw std::runtime_error(fieldError(where + ".level", "levels must be sequential from 1"));
            rule.feeCopper = levels[i].at("feeCopper").integer();
            if (rule.feeCopper < 0 || rule.feeCopper > 100000000)
                throw std::runtime_error(fieldError(where + ".feeCopper", "outside 0..100000000"));
            const auto& materials = levels[i].at("materials").array();
            if (materials.empty())
                throw std::runtime_error(fieldError(where + ".materials", "at least one material is required"));
            for (size_t k = 0; k < materials.size(); ++k) {
                const std::string materialWhere = where + ".materials[" + std::to_string(k) + "]";
                UpgradeMaterial material;
                material.id = materials[k].at("item").string();
                PlayerInventory::ItemType type;
                size_t index = 0;
                if (!catalog.resolveItemId(material.id, type, index))
                    throw std::runtime_error(fieldError(materialWhere + ".item", "unknown item id"));
                material.quantity = materials[k].at("quantity").integer();
                if (material.quantity < 1 || material.quantity > 1000)
                    throw std::runtime_error(fieldError(materialWhere + ".quantity", "must be between 1 and 1000"));
                if (std::any_of(rule.materials.begin(), rule.materials.end(),
                                [&](const UpgradeMaterial& existing) { return existing.id == material.id; }))
                    throw std::runtime_error(fieldError(materialWhere + ".item", "duplicate material id"));
                rule.materials.push_back(std::move(material));
            }
            levels_.push_back(std::move(rule));
        }
    } catch (const std::exception& e) {
        error_ = path.filename().string() + ": " + e.what();
        levels_.clear();
        return;
    }
    loaded_ = true;
}

const UpgradeRule* EquipmentUpgradeDatabase::ruleFor(int targetLevel) const {
    for (const auto& rule : levels_)
        if (rule.level == targetLevel) return &rule;
    return nullptr;
}

const EquipmentUpgradeDatabase& equipmentUpgradeDatabase() {
    static EquipmentUpgradeDatabase database;
    return database;
}

long long equipmentPriceCopper(const EquipmentandWeaponDatabase::equipmentProperties& item) {
    return static_cast<long long>(item.pricePlatinum) * 1000000LL +
           static_cast<long long>(item.priceGold) * 10000LL +
           static_cast<long long>(item.priceSilver) * 100LL +
           item.priceCopper;
}

long long upgradeFeeCopper(const PlayerInventory::InventoryItem& item) {
    const auto* rule = equipmentUpgradeDatabase().ruleFor(item.upgradeLevel + 1);
    return rule ? rule->feeCopper : 0;
}

long long repairCostCopper(const PlayerInventory::InventoryItem& item) {
    if (!isEquipment(item)) return 0;
    const int missing = PlayerInventory::maxDurability - std::clamp(item.durability, 0,
                                                                    PlayerInventory::maxDurability);
    if (missing <= 0) return 0;
    EquipmentandWeaponDatabase database;
    const long long price = toCopper(item, database);
    const long long perPoint = std::max<long long>(1, price / 2000);
    return perPoint * missing;
}

std::string upgradeMaterialLabel(const PlayerInventory& inventory, const UpgradeRule& rule) {
    std::string result;
    for (const auto& material : rule.materials) {
        PlayerInventory::ItemType type;
        size_t index = 0;
        std::string name = material.id;
        if (inventory.resolveItemId(material.id, type, index))
            name = inventory.itemName(type, index);
        if (!result.empty()) result += ", ";
        result += std::to_string(material.quantity) + " " + name;
    }
    return result;
}

std::string repairCostLabel(long long copper) {
    return copperLabel(copper);
}

bool upgradeEquipmentItem(GameSession& session, size_t slot, std::string& error) {
    error.clear();
    if (slot >= session.inventory.inventory.size()) {
        error = "Invalid equipment slot.";
        return false;
    }
    const auto& source = session.inventory.inventory[slot];
    if (!isEquipment(source)) {
        error = "Only equipment can be upgraded.";
        return false;
    }
    const auto* rule = equipmentUpgradeDatabase().ruleFor(source.upgradeLevel + 1);
    if (!rule) {
        error = "This item is already at maximum upgrade level.";
        return false;
    }

    auto result = session.inventory;
    const size_t dbIndex = source.dbIndex;
    for (const auto& material : rule->materials) {
        PlayerInventory::ItemType type;
        size_t index = 0;
        if (!result.resolveItemId(material.id, type, index)) {
            error = "Unknown upgrade material: " + material.id + ".";
            return false;
        }
        if (!result.removeItem(type, index, material.quantity)) {
            error = "Not enough " + result.itemName(type, index) + " for the upgrade.";
            return false;
        }
    }
    auto target = std::find_if(result.inventory.begin(), result.inventory.end(),
                               [&](const PlayerInventory::InventoryItem& entry) {
                                   return entry.type == PlayerInventory::ItemType::Equipment &&
                                          entry.dbIndex == dbIndex;
                               });
    if (target == result.inventory.end()) {
        error = "The equipment disappeared during the upgrade.";
        return false;
    }
    if (!session.hero.economy.subtractCurrency(0, 0, 0, static_cast<int>(rule->feeCopper))) {
        error = "Need " + copperLabel(rule->feeCopper) + " for the upgrade fee.";
        return false;
    }
    target->upgradeLevel++;
    session.inventory = std::move(result);
    session.inventory.refreshUpgrade(session.hero, dbIndex);
    return true;
}

bool repairEquipmentItem(GameSession& session, size_t slot, std::string& error) {
    error.clear();
    if (slot >= session.inventory.inventory.size()) {
        error = "Invalid equipment slot.";
        return false;
    }
    auto& item = session.inventory.inventory[slot];
    if (!isEquipment(item)) {
        error = "Only equipment can be repaired.";
        return false;
    }
    if (item.durability >= PlayerInventory::maxDurability) {
        error = "That item is already in full repair.";
        return false;
    }
    const long long cost = repairCostCopper(item);
    if (!session.hero.economy.subtractCurrency(0, 0, 0, static_cast<int>(cost))) {
        error = "Need " + copperLabel(cost) + " for repairs.";
        return false;
    }
    item.durability = PlayerInventory::maxDurability;
    return true;
}

void openUpgradeMenu(GameSession& session) {
    const auto& database = equipmentUpgradeDatabase();
    if (!database.loaded()) {
        display::message("Upgrade data failed to load: " + database.error());
        return;
    }
    std::vector<size_t> available;
    std::vector<std::string> lines;
    for (size_t i = 0; i < session.inventory.inventory.size(); ++i) {
        const auto& item = session.inventory.inventory[i];
        if (!isEquipment(item)) continue;
        const auto* rule = database.ruleFor(item.upgradeLevel + 1);
        if (!rule) continue;
        available.push_back(i);
        lines.push_back(itemLabel(session.inventory, item) +
                        " -> +" + std::to_string(rule->level) + " (" +
                        upgradeMaterialLabel(session.inventory, *rule) + ", " +
                        repairCostLabel(rule->feeCopper) + ")");
    }
    if (available.empty()) {
        display::message("Nothing can be upgraded.");
        return;
    }
    PagedSelector selector(lines, 5, "Upgrade Equipment");
    const int choice = selector.select();
    if (choice < 0) return;
    std::string error;
    if (upgradeEquipmentItem(session, available[static_cast<size_t>(choice)], error))
        display::message("Upgrade complete.");
    else
        display::message(error);
}

void openRepairMenu(GameSession& session) {
    std::vector<size_t> available;
    std::vector<std::string> lines;
    for (size_t i = 0; i < session.inventory.inventory.size(); ++i) {
        const auto& item = session.inventory.inventory[i];
        if (!isEquipment(item) || item.durability >= PlayerInventory::maxDurability) continue;
        available.push_back(i);
        lines.push_back(itemLabel(session.inventory, item) +
                        " (" + std::to_string(item.durability) + "/" +
                        std::to_string(PlayerInventory::maxDurability) + ", " +
                        repairCostLabel(repairCostCopper(item)) + ")");
    }
    if (available.empty()) {
        display::message("Nothing needs repair.");
        return;
    }
    PagedSelector selector(lines, 5, "Repair Equipment");
    const int choice = selector.select();
    if (choice < 0) return;
    std::string error;
    if (repairEquipmentItem(session, available[static_cast<size_t>(choice)], error))
        display::message("Repairs complete.");
    else
        display::message(error);
}
