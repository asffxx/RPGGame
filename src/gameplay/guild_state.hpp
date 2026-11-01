#pragma once

#include <set>
#include <string>
#include <vector>

// An entry means the guild hall has been discovered. Stages count claimed quests.
struct GuildProgress {
    std::string guildId;
    bool joined = false;
    int completedQuests = 0;
    bool active = false;
    bool objectiveComplete = false;
};

struct RumorHistory {
    std::string tavern;
    std::string rumorId;
};

struct GuildState {
    std::vector<GuildProgress> guilds;
    std::set<std::string> cityLeads;
    std::vector<RumorHistory> rumors;
};
