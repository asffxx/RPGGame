#pragma once

#include "quest_types.hpp"
#include "systems/guild_state.hpp"

struct GameSession;
using GuildQuest = QuestDefinition;

std::vector<GuildQuest> guildQuests(const std::string& guildId);
const GuildQuest* activeGuildQuest(const GuildState& state, const std::string& guildId);
bool guildQuestCompleted(const GuildState& state, const std::string& questId);
bool acceptGuildQuest(GameSession& session, const std::string& guildId);
bool completeGuildObjective(GameSession& session, const std::string& guildId, bool victorious);
bool claimGuildReward(GameSession& session, const std::string& guildId);
