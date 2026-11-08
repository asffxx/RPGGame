#include "story_board.hpp"

#include <algorithm>
#include <set>
#include <sstream>
#include <stdexcept>

#include "app/display.hpp"
#include "app/session.hpp"
#include "core/ui.hpp"
#include "catalogs/content_ids.hpp"
#include "catalogs/content_paths.hpp"
#include "catalogs/guild.hpp"
#include "gameplay/datafile.hpp"
#include "world/dialogue.hpp"
#include "world/quest_system.hpp"

namespace {
using Value = datafile::Value;

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
void fields(const Value& value, const std::set<std::string>& allowed) {
    for (const auto& field : value.object())
        require(allowed.count(field.first), "unknown field: " + field.first);
}
std::string text(const Value& value, const std::string& key) {
    auto result = value.at(key).string();
    require(!result.empty() && result.size() <= 4096, key + ": expected nonempty text (max 4096 bytes)");
    return result;
}
std::string identifier(const Value& value, const std::string& key) {
    auto result = text(value, key);
    require(ids::valid(result), key + ": invalid id " + result);
    return result;
}
Value read(const std::filesystem::path& path) {
    Value value;
    std::string error;
    if (!datafile::loadFile(path, value, error)) throw std::runtime_error(error);
    return value;
}
}

bool StoryDatabase::load(const std::filesystem::path& root, std::string& error) {
    loaded_ = false;
    acts_.clear();
    error_.clear();
    error.clear();
    const auto where = (root / "story/acts.json").string();
    try {
        const auto document = read(where);
        fields(document, {"acts"});
        require(document.at("acts").size() > 0 && document.at("acts").size() <= 8, "expected 1..8 acts");
        PlayerInventory inventory;
        std::set<std::string> actIds;
        for (size_t index = 0; index < document.at("acts").array().size(); ++index) {
            const auto& value = document.at("acts").array()[index];
            fields(value, {"id", "number", "title", "summary", "completion", "beats"});
            StoryAct act;
            act.id = identifier(value, "id");
            require(actIds.insert(act.id).second, "duplicate act id: " + act.id);
            require(value.has("number"), "act requires number");
            act.number = value.at("number").integer();
            require(act.number == static_cast<int>(index) + 1, "acts must be numbered in order");
            act.title = text(value, "title");
            act.summary = text(value, "summary");
            act.completion = text(value, "completion");
            require(value.at("beats").size() > 0 && value.at("beats").size() <= 24, "expected 1..24 beats");
            std::set<std::string> beatIds;
            for (const auto& beatValue : value.at("beats").array()) {
                fields(beatValue, {"id", "title", "text", "hint", "conditions"});
                StoryBeat beat;
                beat.id = identifier(beatValue, "id");
                require(beatIds.insert(beat.id).second, "duplicate beat id: " + beat.id);
                beat.title = text(beatValue, "title");
                beat.text = text(beatValue, "text");
                beat.hint = text(beatValue, "hint");
                require(beatValue.has("conditions") && beatValue.at("conditions").size() > 0 &&
                        beatValue.at("conditions").size() <= 16, "expected 1..16 beat conditions");
                for (const auto& ruleValue : beatValue.at("conditions").array()) {
                    auto rule = parseRule(ruleValue, false, inventory);
                    require(rule.type != "choice", "beat conditions cannot depend on one dialogue graph");
                    beat.conditions.push_back(std::move(rule));
                }
                act.beats.push_back(std::move(beat));
            }
            acts_.push_back(std::move(act));
        }
        loaded_ = true;
        return true;
    } catch (const std::exception& e) {
        error_ = where + ": " + e.what();
        error = error_;
        acts_.clear();
        return false;
    }
}

const StoryDatabase& storyDatabase() {
    static const StoryDatabase database = [] {
        StoryDatabase result;
        std::string error;
        result.load(paths::dataDir(), error);
        return result;
    }();
    return database;
}

StoryView buildStoryView(const GameSession& session) {
    StoryView view;
    const auto& acts = storyDatabase().acts();
    if (acts.empty()) return view;
    view.act = &acts.front();
    view.beats.reserve(view.act->beats.size());
    view.choices.reserve(session.story.choices.size());

    bool previousComplete = true;
    bool currentSeen = false;
    for (const auto& beat : view.act->beats) {
        const bool complete = previousComplete &&
            std::all_of(beat.conditions.begin(), beat.conditions.end(),
                        [&](const ConditionRule& rule) { return conditionPasses(session, "", rule); });
        BeatStatus status = BeatStatus::Later;
        if (complete) status = BeatStatus::Complete;
        else if (!currentSeen) {
            status = BeatStatus::Current;
            currentSeen = true;
        }
        view.beats.push_back({&beat, status});
        previousComplete = complete;
    }
    view.actComplete = !view.beats.empty() && previousComplete;

    for (const auto& quest : allQuests()) {
        if (questProgress(session, quest.id).active)
            view.threads.push_back("Quest: " + quest.title + " [" + questStatusLabel(session, quest) + "]");
    }
    for (const auto& npc : dialogueDatabase().residents()) {
        if (npc.concern.empty() || !cityVisited(session, npc.city)) continue;
        const auto* flag = session.story.flag(npc.concernFlag);
        if (flag && flag->value != 0) continue;
        view.threads.push_back(npc.city + ": " + npc.name + " - " + npc.concern);
    }
    for (const auto& guild : guildDatabase().getGuilds()) {
        const int value = session.story.standing(guild.id);
        view.standings.push_back({guild.city, guild.name, value, tierFor(value)});
    }
    for (const auto& choice : session.story.choices) view.choices.push_back(choice.result);
    return view;
}

void openStoryBoard(const GameSession& session) {
    const auto view = buildStoryView(session);
    clearScreen();
    if (!view.act) {
        display::message("Story data is unavailable.");
        return;
    }
    std::vector<std::string> lines = {"=== Story Board ===",
        "Act " + std::to_string(view.act->number) + ": " + view.act->title,
        view.act->summary, ""};
    size_t complete = 0;
    for (const auto& beat : view.beats) if (beat.status == BeatStatus::Complete) ++complete;
    lines.push_back("Progress: " + std::to_string(complete) + "/" + std::to_string(view.beats.size()) + " beats");
    lines.push_back("");
    for (const auto& beat : view.beats) {
        const char* marker = beat.status == BeatStatus::Complete ? "[x] " :
            beat.status == BeatStatus::Current ? "[>] " : "[ ] ";
        lines.push_back(marker + beat.beat->title);
        lines.push_back(beat.status == BeatStatus::Complete ? "    " + beat.beat->text :
                        "    Hint: " + beat.beat->hint);
    }
    if (view.actComplete) {
        lines.push_back("");
        lines.push_back(view.act->completion);
    }
    lines.push_back("");
    lines.push_back("--- Open Threads ---");
    if (view.threads.empty()) lines.push_back("No open threads.");
    else lines.insert(lines.end(), view.threads.begin(), view.threads.end());
    lines.push_back("");
    lines.push_back("--- Standing ---");
    if (view.standings.empty()) lines.push_back("No guilds known.");
    else for (const auto& standing : view.standings)
        lines.push_back(standing.city + " - " + standing.factionName + ": " +
                        tierName(standing.tier) + " (" + std::to_string(standing.value) + ")");
    lines.push_back("");
    lines.push_back("--- Choices Remembered ---");
    if (view.choices.empty()) lines.push_back("No choices recorded yet.");
    else lines.insert(lines.end(), view.choices.begin(), view.choices.end());
    display::message(lines);
}
