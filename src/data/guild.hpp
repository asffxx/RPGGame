#pragma once

#include <string>
#include <vector>
#include "world/quest_types.hpp"

struct GuildDefinition {
    std::string id;
    std::string name;
    std::string city;
    std::string introduction;
    std::vector<QuestDefinition> quests;
};

class GuildDatabase {
public:
    GuildDatabase();
    const std::vector<GuildDefinition>& getGuilds() const { return guilds; }
    const GuildDefinition* find(const std::string& id) const;
    const GuildDefinition* inCity(const std::string& city) const;
private:
    std::vector<GuildDefinition> guilds;
};

const GuildDatabase& guildDatabase();
