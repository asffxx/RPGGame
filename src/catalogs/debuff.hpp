#pragma once

#include <string>
#include <vector>

#include "core/types.hpp"

// Static catalog of debuffs.
class DebuffDatabase {
public:
    struct DebuffDefinition {
        std::string id{};
        std::string name;
        debuffSettings::DebuffEffect effect;
        debuffSettings::DebuffStackRules stacking;
        debuffSettings::DebuffApplication application;
        std::string effectDesc;
    };

    DebuffDatabase();

    const std::vector<DebuffDefinition>& getDebuffs() const {
        return debuffList;
    }

    // Lookup by id or display name. Names remain the reference used by spells,
    // weapons, enemies, and saves.
    const DebuffDefinition* find(const std::string& nameOrId) const;

private:
    std::vector<DebuffDefinition> debuffList;
};
