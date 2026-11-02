#include "guild.hpp"
#include "content.hpp"

GuildDatabase::GuildDatabase()
    : guilds(content::catalog<GuildDefinition>("narrative/guilds.json")) {}


const GuildDefinition* GuildDatabase::find(const std::string& id) const {
    for (const auto& guild : guilds) if (guild.id == id) return &guild;
    return nullptr;
}

const GuildDefinition* GuildDatabase::inCity(const std::string& city) const {
    for (const auto& guild : guilds) if (guild.city == city) return &guild;
    return nullptr;
}

const GuildDatabase& guildDatabase() {
    static const GuildDatabase database;
    return database;
}
