#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "world/conditions.hpp"

struct GameSession;

struct StoryBeat {
    std::string id;
    std::string title;
    std::string text;
    std::string hint;
    std::vector<ConditionRule> conditions;
};

struct StoryAct {
    std::string id;
    int number = 0;
    std::string title;
    std::string summary;
    std::string completion;
    std::vector<StoryBeat> beats;
};

class StoryDatabase {
public:
    bool load(const std::filesystem::path& root, std::string& error);
    bool loaded() const { return loaded_; }
    const std::string& error() const { return error_; }
    const std::vector<StoryAct>& acts() const { return acts_; }
private:
    bool loaded_ = false;
    std::string error_;
    std::vector<StoryAct> acts_;
};

enum class BeatStatus { Complete, Current, Later };

struct BeatView {
    const StoryBeat* beat = nullptr;
    BeatStatus status = BeatStatus::Later;
};

struct StoryView {
    const StoryAct* act = nullptr;
    bool actComplete = false;
    std::vector<BeatView> beats;
    std::vector<std::string> threads;
    std::vector<std::string> choices;
};

const StoryDatabase& storyDatabase();
StoryView buildStoryView(const GameSession& session);
void openStoryBoard(const GameSession& session);
