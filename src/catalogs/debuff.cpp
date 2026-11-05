#include "debuff.hpp"
#include "content.hpp"

DebuffDatabase::DebuffDatabase()
    : debuffList(content::catalog<DebuffDefinition>("world/debuffs.json")) {}

const DebuffDatabase::DebuffDefinition* DebuffDatabase::find(const std::string& nameOrId) const {
    for (const auto& debuff : debuffList)
        if (debuff.name == nameOrId || debuff.id == nameOrId) return &debuff;
    return nullptr;
}
