#include "guild.hpp"
#include "world/guild_quest.hpp"

GuildDatabase::GuildDatabase() {
    guilds = {
        {"ironward", "Ironward Company", "Attania",
         "Captain Sera offers a place beside the city's roadwardens. 'Bring our people home.'", guildQuests("ironward")},
        {"lantern", "Lantern Scholars", "Erden",
         "Archivist Iven welcomes curious minds. 'A lost page can be more dangerous than a drawn blade.'", guildQuests("lantern")},
        {"silverthread", "Silverthread Consortium", "Trisselie",
         "Factor Mirelle studies your hands. 'Honest work pays. Finding what was stolen pays better.'", guildQuests("silverthread")},
        {"crownward", "Crownward Order", "The Royal Capital",
         "Warden Alis lowers her voice. 'Service to the crown begins with keeping a promise.'", guildQuests("crownward")},
        {"tidebound", "Tidebound Fellowship", "Velmora",
         "Navigator Oren spreads a weathered chart. 'Every lost voyage deserves someone to follow its wake.'", guildQuests("tidebound")}
    };
}

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
