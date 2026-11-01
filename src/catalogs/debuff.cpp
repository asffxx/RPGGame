#include "debuff.hpp"
#include "content.hpp"

DebuffDatabase::DebuffDatabase()
    : debuffList(content::catalog<DebuffDefinition>("world/debuffs.json")) {}
