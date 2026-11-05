#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "catalogs/equipment.hpp"
#include "gameplay/inventory.hpp"

struct GameSession;

// One material required by an equipment upgrade level.
struct UpgradeMaterial {
    std::string id;
    int quantity = 1;
};

// One upgrade step. Levels start at 1; the target level is the rule level.
struct UpgradeRule {
    int level = 0;
    int feeCopper = 0;
    std::vector<UpgradeMaterial> materials;
};

// Authored in content/items/upgrades.json.
class EquipmentUpgradeDatabase {
public:
    EquipmentUpgradeDatabase();

    bool loaded() const { return loaded_; }
    const std::string& error() const { return error_; }
    const std::vector<UpgradeRule>& rules() const { return levels_; }
    int maxLevel() const { return static_cast<int>(levels_.size()); }
    const UpgradeRule* ruleFor(int targetLevel) const;

private:
    std::vector<UpgradeRule> levels_;
    bool loaded_ = false;
    std::string error_;
};

const EquipmentUpgradeDatabase& equipmentUpgradeDatabase();

// Copper value of an item's shop price.
long long equipmentPriceCopper(const EquipmentandWeaponDatabase::equipmentProperties& item);

// Cost to take an item from its current level to the next one.
long long upgradeFeeCopper(const PlayerInventory::InventoryItem& item);

// Cost to restore one item to full durability. Damaged items cost more.
long long repairCostCopper(const PlayerInventory::InventoryItem& item);

// Atomic upgrade/repair of one inventory slot. On failure the session is
// unchanged and `error` explains why.
bool upgradeEquipmentItem(GameSession& session, size_t slot, std::string& error);
bool repairEquipmentItem(GameSession& session, size_t slot, std::string& error);

// Interactive forge/store services.
void openUpgradeMenu(GameSession& session);
void openRepairMenu(GameSession& session);

// Human-readable material list for menus, e.g. "3 Iron Ore, 1 Ember Core".
std::string upgradeMaterialLabel(const PlayerInventory& inventory, const UpgradeRule& rule);
std::string repairCostLabel(long long copper);
