#include "guild_quest.hpp"

#include <map>
#include "app/session.hpp"
#include "catalogs/guild.hpp"
#include "gameplay/guild.hpp"
#include "quest_system.hpp"

std::vector<GuildQuest> guildQuests(const std::string& guildId) {
    const auto* guild = guildDatabase().find(guildId);
    return guild ? guild->quests : std::vector<GuildQuest>{};
}

const GuildQuest* activeGuildQuest(const GuildState& state, const std::string& guildId) {
    const auto* guild = guildDatabase().find(guildId);
    const auto* progress = guildProgress(state, guildId);
    if (!guild || !progress || !progress->active || progress->completedQuests < 0 ||
        progress->completedQuests >= static_cast<int>(guild->quests.size())) return nullptr;
    return &guild->quests[progress->completedQuests];
}

bool guildQuestCompleted(const GuildState& state, const std::string& questId) {
    for (const auto& guild : guildDatabase().getGuilds()) {
        const auto* progress = guildProgress(state, guild.id);
        if (!progress || !progress->joined) continue;
        for (size_t i = 0; i < guild.quests.size(); ++i)
            if (guild.quests[i].id == questId) return static_cast<int>(i) < progress->completedQuests;
    }
    return false;
}

bool acceptGuildQuest(GameSession& session, const std::string& guildId) {
    const auto* guild = guildDatabase().find(guildId);
    const auto* progress = guildProgress(session.guilds, guildId);
    if (!guild || !progress || progress->completedQuests < 0 ||
        progress->completedQuests >= static_cast<int>(guild->quests.size())) return false;
    return acceptQuest(session, guild->quests[progress->completedQuests].id);
}

bool completeGuildObjective(GameSession& session, const std::string& guildId, bool victorious) {
    const auto* quest = activeGuildQuest(session.guilds, guildId);
    return quest && completeQuestObjective(session, quest->id, victorious);
}

bool claimGuildReward(GameSession& session, const std::string& guildId) {
    const auto* quest = activeGuildQuest(session.guilds, guildId);
    return quest && claimQuestReward(session, quest->id);
}
