#pragma once

#include <string>
#include <vector>

enum class QuestCategory { Guild, Main, Side, Commission };

struct QuestDefinition {
    std::string id;
    std::string title;
    std::string briefing;
    std::string location;
    std::string enemyName; // Empty for an investigation/retrieval scene.
    std::string completionText;
    int rewardSilver;
    int rewardExperience;
    QuestCategory category = QuestCategory::Guild;
    int minimumLevel = 1;
    std::string offeringGuild{}; // Empty means any guild hall.
    std::string prerequisite{}; // A guild quest whose reward must have been claimed.
};

struct QuestProgress {
    std::string questId;
    bool active = false;
    bool objectiveComplete = false;
    int completions = 0;
    std::string acceptingGuild{};
};

struct QuestState {
    // An entry also records discovery. Guild storylines keep their legacy stage state.
    std::vector<QuestProgress> jobs;
};
