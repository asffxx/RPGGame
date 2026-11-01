#include "quest_system.hpp"

#include <algorithm>
#include <stdexcept>
#include "app/display.hpp"
#include "app/session.hpp"
#include "combat/combat.hpp"
#include "core/ui.hpp"
#include "data/guild.hpp"
#include "systems/guild.hpp"
#include "ui/player_controller.hpp"
#include "comissions.hpp"
#include "encounters.hpp"
#include "guild_quest.hpp"
#include "quest.hpp"
#include "sidequests.hpp"

namespace {
constexpr int maxCompletions = 1000000;

const GuildDefinition* currentGuild(const GameSession& session) {
    return session.hero.currentLocationType == PeacefulCity ? guildDatabase().inCity(session.hero.currentLocation) : nullptr;
}

QuestProgress* storedProgress(GameSession& session, const std::string& id) {
    for (auto& progress : session.quests.jobs) if (progress.questId == id) return &progress;
    return nullptr;
}

int guildQuestIndex(const QuestDefinition& quest) {
    const auto* guild = guildDatabase().find(quest.offeringGuild);
    if (guild) for (size_t i = 0; i < guild->quests.size(); ++i)
        if (guild->quests[i].id == quest.id) return static_cast<int>(i);
    return -1;
}

std::string returnInstructions(const QuestDefinition& quest) {
    if (quest.offeringGuild.empty()) return "Return to any guild hall for your reward.";
    return "Return to " + guildDatabase().find(quest.offeringGuild)->city + " for your reward.";
}

std::string statusLabel(const GameSession& session, const QuestDefinition& quest) {
    const auto progress = questProgress(session, quest.id);
    if (progress.objectiveComplete) return "Ready to turn in";
    if (progress.active) return "Active";
    if (progress.completions && quest.category != QuestCategory::Commission) return "Completed";
    auto reason = questLockReason(session, quest);
    if (!reason.empty()) return "Locked: " + reason;
    return quest.category == QuestCategory::Commission && progress.completions ? "Available again" : "Available";
}
}

const char* questCategoryName(QuestCategory category) {
    switch (category) {
        case QuestCategory::Guild: return "Guild quests";
        case QuestCategory::Main: return "Main quests";
        case QuestCategory::Side: return "Side quests";
        case QuestCategory::Commission: return "Commissions";
    }
    return "Unknown";
}

const std::vector<QuestDefinition>& allQuests() {
    static const std::vector<QuestDefinition> catalog = [] {
        std::vector<QuestDefinition> result;
        for (const auto& guild : guildDatabase().getGuilds())
            result.insert(result.end(), guild.quests.begin(), guild.quests.end());
        for (const auto* entries : {&mainQuests(), &sideQuests(), &commissions()})
            result.insert(result.end(), entries->begin(), entries->end());
        return result;
    }();
    return catalog;
}

const QuestDefinition* findQuest(const std::string& id) {
    for (const auto& quest : allQuests()) if (quest.id == id) return &quest;
    return nullptr;
}

QuestProgress questProgress(const GameSession& session, const std::string& id) {
    const auto* quest = findQuest(id);
    if (quest && quest->category == QuestCategory::Guild) {
        const auto* progress = guildProgress(session.guilds, quest->offeringGuild);
        const int index = guildQuestIndex(*quest);
        if (progress && index >= 0) {
            bool current = progress->completedQuests == index;
            return {id, current && progress->active, current && progress->objectiveComplete,
                    progress->completedQuests > index ? 1 : 0, quest->offeringGuild};
        }
    } else {
        for (const auto& progress : session.quests.jobs) if (progress.questId == id) return progress;
    }
    return {id};
}

std::vector<const QuestDefinition*> boardQuests(const GameSession& session, QuestCategory category) {
    std::vector<const QuestDefinition*> result;
    const auto* guild = currentGuild(session);
    if (!guild || (category != QuestCategory::Side && category != QuestCategory::Commission)) return result;
    for (const auto& quest : allQuests())
        if (quest.category == category && (quest.offeringGuild.empty() || quest.offeringGuild == guild->id)) result.push_back(&quest);
    return result;
}

void discoverBoardQuests(GameSession& session, QuestCategory category) {
    for (const auto* quest : boardQuests(session, category))
        if (!storedProgress(session, quest->id)) session.quests.jobs.push_back({quest->id});
}

std::string questLockReason(const GameSession& session, const QuestDefinition& quest) {
    std::vector<std::string> reasons;
    if (session.hero.stats.level < quest.minimumLevel) reasons.push_back("requires level " + std::to_string(quest.minimumLevel));
    if (!quest.offeringGuild.empty()) {
        const auto* guild = guildDatabase().find(quest.offeringGuild);
        const auto* progress = guildProgress(session.guilds, quest.offeringGuild);
        if (!progress || !progress->joined) reasons.push_back("join " + (guild ? guild->name : quest.offeringGuild));
        if (quest.category == QuestCategory::Guild && progress && progress->completedQuests != guildQuestIndex(quest))
            reasons.push_back("follow the guild storyline in order");
    }
    if (!quest.prerequisite.empty() && !guildQuestCompleted(session.guilds, quest.prerequisite)) {
        const auto* required = findQuest(quest.prerequisite);
        reasons.push_back("turn in " + (required ? required->title : quest.prerequisite));
    }
    if (questProgress(session, quest.id).completions >= maxCompletions) reasons.push_back("completion limit reached");
    std::string result;
    for (const auto& reason : reasons) { if (!result.empty()) result += "; "; result += reason; }
    return result;
}

bool acceptQuest(GameSession& session, const std::string& id) {
    const auto* quest = findQuest(id);
    const auto* guild = currentGuild(session);
    if (!quest || !guild || (!quest->offeringGuild.empty() && quest->offeringGuild != guild->id) ||
        !questLockReason(session, *quest).empty()) return false;
    auto previous = questProgress(session, id);
    if (previous.active || (previous.completions && quest->category != QuestCategory::Commission)) return false;
    if (quest->category == QuestCategory::Guild) {
        auto& progress = discoverGuild(session.guilds, guild->id);
        progress.active = true;
        progress.objectiveComplete = false;
    } else {
        auto* progress = storedProgress(session, id);
        if (!progress) { session.quests.jobs.push_back({id}); progress = &session.quests.jobs.back(); }
        progress->active = true;
        progress->objectiveComplete = false;
        progress->acceptingGuild = guild->id;
    }
    session.guilds.cityLeads.insert(guild->city);
    return true;
}

std::vector<std::string> questsAtLocation(const GameSession& session) {
    std::vector<std::string> result;
    if (session.hero.stats.hitpoints <= 0) return result;
    for (const auto& quest : allQuests()) {
        auto progress = questProgress(session, quest.id);
        if (progress.active && !progress.objectiveComplete && quest.location == session.hero.currentLocation) result.push_back(quest.id);
    }
    return result;
}

bool completeQuestObjective(GameSession& session, const std::string& id, bool victorious) {
    const auto* quest = findQuest(id);
    if (!quest || !victorious || session.hero.stats.hitpoints <= 0 || quest->location != session.hero.currentLocation) return false;
    auto progress = questProgress(session, id);
    if (!progress.active || progress.objectiveComplete) return false;
    if (quest->category == QuestCategory::Guild) discoverGuild(session.guilds, quest->offeringGuild).objectiveComplete = true;
    else storedProgress(session, id)->objectiveComplete = true;
    return true;
}

bool claimQuestReward(GameSession& session, const std::string& id) {
    const auto* quest = findQuest(id);
    const auto* guild = currentGuild(session);
    if (!quest || !guild || session.hero.stats.hitpoints <= 0 ||
        (!quest->offeringGuild.empty() && quest->offeringGuild != guild->id)) return false;
    auto progress = questProgress(session, id);
    if (!progress.active || !progress.objectiveComplete || progress.completions >= maxCompletions) return false;
    if (quest->category == QuestCategory::Guild) {
        auto& state = discoverGuild(session.guilds, guild->id);
        state.active = state.objectiveComplete = false;
        ++state.completedQuests;
    } else {
        auto& state = *storedProgress(session, id);
        state.active = state.objectiveComplete = false;
        ++state.completions;
    }
    session.hero.economy.addCurrency(0, 0, quest->rewardSilver, 0);
    session.hero.stats.expe += quest->rewardExperience;
    PlayerController(session.hero, session.inventory).levelUpChecker();
    return true;
}

std::set<std::string> travelLeads(const GameSession& session) {
    auto result = session.guilds.cityLeads;
    for (const auto& quest : allQuests()) {
        auto progress = questProgress(session, quest.id);
        if (progress.active && !progress.objectiveComplete) result.insert(quest.location);
    }
    return result;
}

void validateQuestState(const GameSession& session) {
    auto require = [](bool valid, const char* message) { if (!valid) throw std::runtime_error(message); };
    std::set<std::string> ids;
    for (const auto& progress : session.quests.jobs) {
        const auto* quest = findQuest(progress.questId);
        require(quest && quest->category != QuestCategory::Guild && ids.insert(progress.questId).second, "Unknown or duplicate saved job.");
        require(progress.completions >= 0 && progress.completions <= maxCompletions, "Invalid quest completion count.");
        require(!progress.objectiveComplete || progress.active, "Objective complete on an inactive job.");
        require(quest->category == QuestCategory::Commission || (progress.completions <= 1 && !(progress.active && progress.completions)), "Repeated one-time quest.");
        if (progress.active || progress.completions) {
            const auto* giver = guildDatabase().find(progress.acceptingGuild);
            require(giver && (quest->offeringGuild.empty() || quest->offeringGuild == giver->id), "Invalid accepting guild.");
            require(session.guilds.cityLeads.count(giver->city), "Missing quest return lead.");
            require(session.hero.stats.level >= quest->minimumLevel, "Quest accepted below required level.");
            if (!quest->offeringGuild.empty()) {
                const auto* membership = guildProgress(session.guilds, quest->offeringGuild);
                require(membership && membership->joined, "Guild job without membership.");
            }
            require(quest->prerequisite.empty() || guildQuestCompleted(session.guilds, quest->prerequisite), "Missing quest prerequisite.");
        } else require(progress.acceptingGuild.empty(), "Unaccepted job has a giver.");
        require(!progress.active || progress.completions < maxCompletions, "Quest active beyond completion limit.");
    }
}

void openQuestBoard(GameSession& session, QuestCategory category) {
    if (!currentGuild(session)) return;
    discoverBoardQuests(session, category);
    while (true) {
        clearScreen();
        auto quests = boardQuests(session, category);
        std::vector<std::string> lines = {std::string("=== ") + questCategoryName(category) + " ==="};
        for (size_t i = 0; i < quests.size(); ++i)
            lines.push_back(std::to_string(i + 1) + ". " + quests[i]->title + " [" + statusLabel(session, *quests[i]) + "]");
        lines.push_back("0. Back to guild hall");
        displayBorderedMenu(lines, "Choose a job: ");
        int choice = getNumberInput(0, static_cast<int>(quests.size()));
        if (choice == 0) return;
        const auto& quest = *quests[choice - 1];
        auto progress = questProgress(session, quest.id);
        lines = {quest.title, quest.briefing, returnInstructions(quest),
            "Reward: " + std::to_string(quest.rewardSilver) + " silver, " + std::to_string(quest.rewardExperience) + " XP.",
            "Status: " + statusLabel(session, quest)};
        bool canAccept = !progress.active && (!progress.completions || category == QuestCategory::Commission) && questLockReason(session, quest).empty();
        if (canAccept || progress.objectiveComplete) {
            lines.push_back(progress.objectiveComplete ? "1. Turn in quest" : "1. Accept quest");
            lines.push_back("2. Back");
            displayBorderedMenu(lines, "Choose an option: ");
            if (getNumberInput(1, 2) == 2) continue;
            bool changed = progress.objectiveComplete ? claimQuestReward(session, quest.id) : acceptQuest(session, quest.id);
            display::message(changed ? (progress.objectiveComplete ? "Quest turned in. Reward received." : "Quest accepted. Use Travel -> Follow a lead.") : "You cannot do that right now.");
        } else display::message(lines);
    }
}

void showQuestJournal(const GameSession& session) {
    std::vector<std::string> lines = {"=== Quest Journal ==="};
    for (auto category : {QuestCategory::Guild, QuestCategory::Main, QuestCategory::Side, QuestCategory::Commission}) {
        lines.push_back(std::string("--- ") + questCategoryName(category) + " ---");
        bool any = false;
        for (const auto& quest : allQuests()) {
            if (quest.category != category) continue;
            bool discovered = category == QuestCategory::Guild ? guildProgress(session.guilds, quest.offeringGuild) != nullptr :
                std::any_of(session.quests.jobs.begin(), session.quests.jobs.end(), [&](const auto& p) { return p.questId == quest.id; });
            if (!discovered) continue;
            any = true;
            auto progress = questProgress(session, quest.id);
            lines.push_back(quest.title + " [" + statusLabel(session, quest) + "]");
            if (progress.active) lines.push_back(progress.objectiveComplete ? returnInstructions(quest) : quest.briefing);
            if (category == QuestCategory::Commission) lines.push_back("Runs completed: " + std::to_string(progress.completions));
        }
        if (!any) lines.push_back(category == QuestCategory::Main ? "No main quests available yet." : "None discovered. Visit a guild hall.");
    }
    for (const auto& progress : session.guilds.guilds) {
        const auto& guild = *guildDatabase().find(progress.guildId);
        if (progress.completedQuests == static_cast<int>(guild.quests.size())) lines.push_back(guild.name + ": Storyline complete.");
    }
    lines.push_back("Known city leads:");
    for (const auto& city : session.guilds.cityLeads)
        lines.push_back(city + (session.travel.discovered.count(city) ? " [visited]" : " [not visited]"));
    if (session.guilds.cityLeads.empty()) lines.push_back("None yet. Ask at a tavern for rumors.");
    display::message(lines);
}

void performQuestAction(GameSession& session, EnemyController& enemies, CombatSystem& combat, NPCGenerator& generator) {
    auto ids = questsAtLocation(session);
    if (ids.empty()) { display::message("No active quest objective here."); return; }
    std::vector<std::string> lines;
    for (const auto& id : ids) {
        const auto& quest = *findQuest(id);
        lines.push_back(std::to_string(lines.size() + 1) + ". [" + questCategoryName(quest.category) + "] " + quest.title +
                        (quest.enemyName.empty() ? " [Investigate]" : " [Fight " + quest.enemyName + "]"));
    }
    lines.push_back("0. Back");
    displayBorderedMenu(lines, "Choose a quest action: ");
    int choice = getNumberInput(0, static_cast<int>(ids.size()));
    if (choice == 0) return;
    const auto& quest = *findQuest(ids[choice - 1]);
    bool victorious = true;
    if (!quest.enemyName.empty()) {
        auto area = currentArea(quest.location);
        Enemy enemy = enemies.getEnemyByName(quest.enemyName, area.difficultyLevel, area.type);
        SpellDatabase spells;
        CombatScreen screen(session.hero, session.party, enemy, session.hero.timeSystem, generator, spells);
        screen.startCombat(combat, session.inventory);
        victorious = session.hero.stats.hitpoints > 0 && enemy.stats.data.hitpoints <= 0;
        if (victorious) {
            enemies.enemyGoldExpDrop(session.hero, enemy);
            PlayerController(session.hero, session.inventory).levelUpChecker();
        }
    }
    if (completeQuestObjective(session, quest.id, victorious)) display::message(quest.completionText + "\n" + returnInstructions(quest));
    else display::message("The objective is still unfinished. You can try again when ready.");
    int hp = session.hero.stats.hitpoints;
    session.hero.timeSystem.advanceTime(session.hero);
    if (hp <= 0) session.hero.stats.hitpoints = hp;
}
