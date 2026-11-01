#include "race.hpp"
#include "content.hpp"

PlayerRaceDatabase::PlayerRaceDatabase()
    : templates(content::catalog<PlayerRaceTemplate>("characters/races.json")) {}

const PlayerRaceTemplate* PlayerRaceDatabase::findById(const std::string& id) const {
    for (const auto& race : templates)
        if (race.id == id) return &race;
    return nullptr;
}
