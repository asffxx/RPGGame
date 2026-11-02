#include "rumor.hpp"
#include "content.hpp"

RumorDatabase::RumorDatabase()
    : rumors(content::catalog<RumorDefinition>("narrative/rumors.json")) {}

const RumorDefinition* RumorDatabase::find(const std::string& id) const {
    for (const auto& rumor : rumors) if (rumor.id == id) return &rumor;
    return nullptr;
}

const RumorDatabase& rumorDatabase() {
    static const RumorDatabase database;
    return database;
}
