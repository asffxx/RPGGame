#include "guild_quest.hpp"

#include <map>
#include "app/session.hpp"
#include "data/guild.hpp"
#include "systems/guild.hpp"
#include "quest_system.hpp"

std::vector<GuildQuest> guildQuests(const std::string& guildId) {
    static const std::map<std::string, std::vector<GuildQuest>> catalog = {
        {"ironward", {
            {"ironward_supplies", "The Silent Patrol", "Recover the missing patrol's supply satchel in Barren Plains.", "Barren Plains", "",
             "Beneath a broken cart you find the patrol's supplies and a scout's map leading to Shadowed Grove.", 40, 30},
            {"ironward_scout", "Tracks in the Grove", "Confront the Goblin Scout in Shadowed Grove and secure the patrol's escape route.", "Shadowed Grove", "Goblin Scout",
             "The scout falls. The surviving wardens emerge from hiding, ready to return to Attania.", 100, 70}
        }},
        {"lantern", {
            {"lantern_pages", "Scattered Pages", "Retrieve the scholar's research from Enchanted Forest.", "Enchanted Forest", "",
             "You gather rain-stained pages beneath a hollow tree. Their diagrams point to Enchanted Library.", 40, 30},
            {"lantern_keeper", "The Unquiet Archive", "Defeat the Bonewalker guarding the missing folio in Enchanted Library.", "Enchanted Library", "Bonewalker",
             "The guardian crumbles, revealing the folio that completes the scholars' research.", 100, 70}
        }},
        {"silverthread", {
            {"silverthread_ledger", "An Unbalanced Ledger", "Recover the stolen shipping ledger hidden in Ghost Town.", "Ghost Town", "",
             "A ledger lies under a loose floorboard. False entries reveal a shipment diverted to Bandit Hideout.", 50, 25},
            {"silverthread_outlaw", "The Last Delivery", "Confront the Outlaw in Bandit Hideout and reclaim the stolen merchandise.", "Bandit Hideout", "Outlaw",
             "Behind the outlaw's barricade you find the missing cargo, still bearing Trisselie's silver seals.", 120, 60}
        }},
        {"crownward", {
            {"crownward_dispatch", "A Courier's Promise", "Find the missing courier's dispatch in Whispering Woods.", "Whispering Woods", "",
             "The wounded courier entrusts you with a dispatch: raiders carried the royal seal to Abandoned Castle.", 40, 35},
            {"crownward_seal", "The Stolen Seal", "Defeat the Goblin Marauder at Abandoned Castle and retrieve the royal seal.", "Abandoned Castle", "Goblin Marauder",
             "The marauder drops a velvet case. The royal seal inside will restore trust in the capital's messengers.", 110, 80}
        }},
        {"tidebound", {
            {"tidebound_journal", "Beyond the Last Chart", "Find the expedition's journal at Desert Oasis.", "Desert Oasis", "",
             "The expedition sheltered here. Its journal marks a relic cache deep in Crystal Caverns.", 45, 30},
            {"tidebound_relic", "A Treasure with Teeth", "Defeat the Mimic concealing the expedition's relic in Crystal Caverns.", "Crystal Caverns", "Mimic",
             "The false chest collapses. You recover the expedition's tideglass relic from its splintered remains.", 100, 75}
        }}
    };
    auto found = catalog.find(guildId);
    if (found == catalog.end()) return {};
    auto result = found->second;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i].offeringGuild = guildId;
        if (i != 0) result[i].prerequisite = result[i - 1].id;
    }
    return result;
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
