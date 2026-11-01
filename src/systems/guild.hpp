#pragma once

#include "app/session.hpp"
#include "data/guild.hpp"
#include "world/guild_quest.hpp"
#include "world/quest_system.hpp"

class CombatSystem;
class EnemyController;

const GuildProgress* guildProgress(const GuildState& state, const std::string& id);
GuildProgress& discoverGuild(GuildState& state, const std::string& id);
bool joinGuild(GameSession& session, const std::string& id);
void validateGuildState(const GameSession& session); // Throws for invalid saved IDs/states.

void openGuildHall(GameSession& session);
