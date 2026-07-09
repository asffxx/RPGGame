#pragma once

#include <string>
#include <vector>

#include "core/types.hpp"

// Static catalog of debuffs.
class DebuffDatabase {
public:
    struct DebuffDefinition {
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

private:
    std::vector<DebuffDefinition> debuffList;
};
