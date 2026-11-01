#pragma once

#include "entities/player.hpp"
#include "entities/npc.hpp"
#include "gameplay/inventory.hpp"
#include "world/travel_state.hpp"
#include "gameplay/guild_state.hpp"
#include "world/quest_types.hpp"
#include "world/story_types.hpp"

// Persistent state, independent of runtime controllers and their references.
struct GameSession {
    explicit GameSession(const Player& player) : hero(player) {}

    Player hero;
    PlayerInventory inventory;
    std::vector<NPC> party;
    TravelState travel;
    GuildState guilds;
    QuestState quests;
    StoryState story;
    int actionCounter = 0;
    int lastWeekPaid = 0;
};
