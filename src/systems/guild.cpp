#include "guild.hpp"

#include <stdexcept>
#include "app/display.hpp"
#include "core/ui.hpp"
#include "data/rumor.hpp"
#include "world/encounters.hpp"

const GuildProgress* guildProgress(const GuildState& state, const std::string& id) {
    for (const auto& progress : state.guilds) if (progress.guildId == id) return &progress;
    return nullptr;
}

GuildProgress& discoverGuild(GuildState& state, const std::string& id) {
    const auto* guild = guildDatabase().find(id);
    if (!guild) throw std::invalid_argument("Unknown guild.");
    state.cityLeads.insert(guild->city); // The hall provides directions for the return journey.
    for (auto& progress : state.guilds) if (progress.guildId == id) return progress;
    state.guilds.push_back({id});
    return state.guilds.back();
}

bool joinGuild(GameSession& session, const std::string& id) {
    const auto* guild = guildDatabase().find(id);
    if (!guild || session.hero.currentLocation != guild->city || session.hero.currentLocationType != PeacefulCity) return false;
    auto& progress = discoverGuild(session.guilds, id);
    if (progress.joined) return false;
    progress.joined = true;
    return true;
}

void validateGuildState(const GameSession& session) {
    auto require = [](bool valid, const char* error) { if (!valid) throw std::runtime_error(error); };
    std::set<std::string> ids;
    for (const auto& p : session.guilds.guilds) {
        const auto* guild = guildDatabase().find(p.guildId);
        require(guild && ids.insert(p.guildId).second, "Unknown or duplicate saved guild.");
        require(p.completedQuests >= 0 && p.completedQuests <= static_cast<int>(guild->quests.size()), "Invalid quest stage.");
        require(p.joined || (p.completedQuests == 0 && !p.active && !p.objectiveComplete), "Quest progress without membership.");
        require(!p.objectiveComplete || p.active, "Completed objective without active quest.");
        require(!p.active || p.completedQuests < static_cast<int>(guild->quests.size()), "Quest active after storyline completion.");
        require(session.guilds.cityLeads.count(guild->city), "Missing guild return route.");
    }
    for (const auto& city : session.guilds.cityLeads)
        require(guildDatabase().inCity(city) != nullptr, "Unknown saved city lead.");
    std::set<std::string> taverns;
    for (const auto& history : session.guilds.rumors) {
        auto area = currentArea(history.tavern);
        require(area.name == history.tavern && (area.type == PeacefulCity || area.type == PeacefulTown) &&
                taverns.insert(history.tavern).second, "Invalid saved rumor tavern.");
        const auto* rumor = rumorDatabase().find(history.rumorId);
        require(rumor != nullptr, "Unknown saved rumor.");
        require(rumor->destination.empty() ? rumor->city == history.tavern : rumor->destination != history.tavern,
                "Rumor does not belong to this tavern.");
        require(rumor->destination.empty() || session.guilds.cityLeads.count(rumor->destination), "Missing rumor destination.");
    }
}

void openGuildHall(GameSession& session) {
    const auto* guild = guildDatabase().inCity(session.hero.currentLocation);
    if (!guild || session.hero.currentLocationType != PeacefulCity) return;
    discoverGuild(session.guilds, guild->id);
    while (true) {
        clearScreen();
        const auto& progress = *guildProgress(session.guilds, guild->id);
        std::vector<std::string> lines = {guild->name + " - " + guild->city, guild->introduction};
        bool finished = progress.completedQuests == static_cast<int>(guild->quests.size());
        if (!progress.joined) lines.push_back("1. Join guild (free; other guilds remain available)");
        else if (finished) lines.push_back("1. Storyline complete. Review your service.");
        else {
            const auto& quest = guild->quests[progress.completedQuests];
            lines.push_back(quest.title + ": " + quest.briefing);
            lines.push_back("Reward: " + std::to_string(quest.rewardSilver) + " silver, " + std::to_string(quest.rewardExperience) + " XP.");
            lines.push_back(progress.objectiveComplete ? "1. Turn in quest" :
                            (progress.active ? "1. Review active quest" : "1. Accept quest"));
        }
        lines.push_back("2. Leave guild hall");
        lines.push_back("3. Side Quests");
        lines.push_back("4. Commissions");
        displayBorderedMenu(lines, "Choose an option: ");
        int choice = getNumberInput(1, 4);
        if (choice == 2) return;
        if (choice == 3 || choice == 4) {
            openQuestBoard(session, choice == 3 ? QuestCategory::Side : QuestCategory::Commission);
            continue;
        }
        if (finished) { display::message("Storyline complete. Your service will be remembered."); continue; }
        if (!progress.joined) {
            joinGuild(session, guild->id);
            display::message("You joined " + guild->name + ". You may also join the other guilds.");
        } else if (progress.objectiveComplete) {
            display::message(claimGuildReward(session, guild->id) ? "Quest turned in. Reward received." : "You must be alive to claim the reward.");
        } else if (!progress.active) {
            acceptGuildQuest(session, guild->id);
            display::message("Quest accepted. Its destination is available under Travel -> Follow a lead.");
        } else display::message("Follow the objective in your Quest Journal, then return here for your reward.");
    }
}
