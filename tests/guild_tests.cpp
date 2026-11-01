#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>

#include "app/main_menu.hpp"
#include "combat/combat.hpp"
#include "data/class.hpp"
#include "data/race.hpp"
#include "systems/guild.hpp"
#include "systems/rumor.hpp"
#include "systems/save.hpp"
#include "ui/player_controller.hpp"
#include "world/encounters.hpp"
#include "world/establishments.hpp"
#include "world/travel.hpp"

namespace {
void check(bool value, const std::string& message) {
    if (!value) throw std::runtime_error(message);
}

GameSession newSession() {
    PlayerClassCollection classes;
    PlayerRaceDatabase races;
    Player hero("Guild Tester", {races.templates[0].name, {races.templates[0].lore.description}}, classes.getClassTemplate("Ranger"));
    hero.applyRaceBonus(races.templates[0].statBonus);
    hero.stats.hitpoints = hero.stats.maxHitpoints = 10000;
    hero.stats.attack = 10000;
    hero.progression.experienceRequired = 1000000;
    return GameSession(hero);
}

void place(GameSession& session, const std::string& name) {
    auto area = currentArea(name);
    check(area.name == name, "Unknown test location");
    session.hero.currentLocation = area.name;
    session.hero.currentLocationType = area.type;
}

long long copper(const GameSession& s) {
    const auto& e = s.hero.economy;
    return ((static_cast<long long>(e.platinum) * 100 + e.gold) * 100 + e.silver) * 100 + e.copper;
}

// Fail quickly on exhausted scripted input rather than hanging a numeric menu.
class MenuIO {
public:
    explicit MenuIO(const std::string& commands) : input(commands), oldIn(std::cin.rdbuf()), oldOut(std::cout.rdbuf()), flags(std::cin.exceptions()) {
        std::cin.clear();
        std::cin.rdbuf(input.rdbuf());
        std::cout.rdbuf(output.rdbuf());
        std::cin.exceptions(std::ios::badbit | std::ios::failbit);
    }
    ~MenuIO() {
        std::cin.exceptions(std::ios::goodbit);
        std::cin.clear();
        std::cin.rdbuf(oldIn);
        std::cout.rdbuf(oldOut);
        std::cin.exceptions(flags);
    }
    std::string text() const { return output.str(); }
private:
    std::istringstream input;
    std::ostringstream output;
    std::streambuf* oldIn;
    std::streambuf* oldOut;
    std::ios::iostate flags;
};

void progression() {
    auto session = newSession();
    check(guildDatabase().getGuilds().size() == 5, "Expected five guilds");
    for (const auto& guild : guildDatabase().getGuilds()) {
        check(guild.quests.size() == 2, "Expected two quests per guild");
        check(!joinGuild(session, guild.id), "Joined a remote guild");
        place(session, guild.city);
        discoverGuild(session.guilds, guild.id);
        check(!acceptGuildQuest(session, guild.id), "Accepted before joining");
        check(joinGuild(session, guild.id), "Could not join guild");
        check(!joinGuild(session, guild.id), "Joined same guild twice");
        check(acceptGuildQuest(session, guild.id), "Could not accept first quest");
        check(!acceptGuildQuest(session, guild.id), "Skipped active quest");
    }
    check(session.guilds.guilds.size() == 5, "Guilds became exclusive");
    for (const auto& guild : guildDatabase().getGuilds()) {
        for (size_t stage = 0; stage < guild.quests.size(); ++stage) {
            const auto& quest = guild.quests[stage];
            if (stage != 0) {
                place(session, guild.city);
                check(acceptGuildQuest(session, guild.id), "Could not accept second quest");
            }
            check(travelLeads(session).count(quest.location), "No objective lead");
            place(session, guild.city);
            check(!completeGuildObjective(session, guild.id, true), "Objective completed in wrong location");
            check(!claimGuildReward(session, guild.id), "Claimed unfinished objective");
            place(session, quest.location);
            check(!completeGuildObjective(session, guild.id, false), "Retreat completed quest");
            session.hero.stats.hitpoints = 0;
            check(!completeGuildObjective(session, guild.id, true), "Dead hero completed quest");
            session.hero.stats.hitpoints = 10000;
            check(completeGuildObjective(session, guild.id, true), "Objective failed to complete");
            check(!completeGuildObjective(session, guild.id, true), "Repeated completed objective");
            check(!claimGuildReward(session, guild.id), "Claimed reward away from guild");
            place(session, guild.city);
            session.hero.stats.hitpoints = 0;
            check(!claimGuildReward(session, guild.id), "Dead hero claimed reward");
            session.hero.stats.hitpoints = 10000;
            auto before = copper(session);
            float xp = session.hero.stats.expe;
            check(claimGuildReward(session, guild.id), "Reward missing");
            check(copper(session) == before + quest.rewardSilver * 100 && session.hero.stats.expe == xp + quest.rewardExperience,
                  "Incorrect quest reward");
            check(!claimGuildReward(session, guild.id), "Duplicate quest reward");
            check(copper(session) == before + quest.rewardSilver * 100, "Duplicate claim changed currency");
        }
        check(!acceptGuildQuest(session, guild.id), "Accepted quest beyond storyline");
    }
    validateGuildState(session);
}

void rumors() {
    auto session = newSession();
    std::mt19937 random(512);
    check(eligibleRumors(session).empty(), "Rumors available outside a tavern");
    place(session, "Peaceful Town");
    float xp = session.hero.stats.expe;
    int turn = session.hero.timeSystem.getCurrentTurn();
    for (int i = 0; i < 5; ++i) {
        auto* rumor = hearRumor(session, random);
        check(rumor && !rumor->destination.empty(), "No actionable city clue");
        check(session.guilds.cityLeads.size() == static_cast<size_t>(i + 1), "Unknown-city preference failed");
        check(rumor->text.find("Ariandel") == std::string::npos, "Royal Capital naming violated");
    }
    check(session.travel.discovered.empty() && session.hero.discoveredLocations.empty() && session.travel.marked.empty(), "Clue counted as visit");
    check(session.hero.stats.expe == xp && session.hero.timeSystem.getCurrentTurn() == turn && copper(session) == 0, "Rumor cost time or currency, or granted XP");
    for (const auto& guild : guildDatabase().getGuilds()) {
        place(session, guild.city);
        auto pool = eligibleRumors(session);
        int locals = 0;
        for (const auto* rumor : pool) {
            check(rumor->city.empty() || rumor->city == guild.city, "Another city's local rumor leaked");
            if (rumor->city == guild.city) ++locals;
        }
        check(locals >= 2, "Missing local rumors");
        std::string previous;
        for (int i = 0; i < 20; ++i) {
            const auto* rumor = hearRumor(session, random);
            check(rumor && rumor->id != previous, "Immediate rumor repeat");
            previous = rumor->id;
        }
    }
    check(travelLeads(session).count("The Royal Capital"), "Capital lead missing");
    check(recordArrival(session, "The Royal Capital"), "First visit not recorded");
    check(session.hero.stats.expe == xp + 50 && session.hero.discoveredLocations.count("The Royal Capital"), "First arrival lost discovery credit");
    check(!recordArrival(session, "The Royal Capital") && session.hero.stats.expe == xp + 50, "Repeated discovery XP");
    check(session.travel.marked.empty(), "Arrival automatically marked a safe route");
    check(encounterChance(PeacefulCity) == 0, "Cities are not peaceful");
    validateGuildState(session);
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
void writeFile(const std::filesystem::path& path, const std::string& data) {
    std::ofstream output(path, std::ios::binary); output << data;
}

void savesTest(const std::filesystem::path& directory) {
    auto session = newSession();
    std::mt19937 random(3);
    place(session, "Peaceful Town");
    const auto* heard = hearRumor(session, random);
    std::string last = heard->id;
    place(session, "Attania");
    joinGuild(session, "ironward");
    acceptGuildQuest(session, "ironward");
    place(session, "Barren Plains");
    completeGuildObjective(session, "ironward", true);
    session.hero.timeSystem.setState(5, TimeSystem::TimePeriod::Afternoon, 1);
    session.actionCounter = 2;
    std::string error;
    auto first = directory / "guild-v2.dat", second = directory / "guild-copy.dat";
    check(saves::save(session, first, error), "Cannot save guilds: " + error);
    auto loaded = saves::load(first, error);
    check(loaded.has_value(), "Cannot load guilds: " + error);
    check(guildProgress(loaded->guilds, "ironward")->objectiveComplete, "Lost ready-to-turn-in quest");
    check(saves::save(*loaded, second, error), "Cannot resave guilds");
    check(readFile(first) == readFile(second), "Guild state did not round trip");
    place(*loaded, "Peaceful Town");
    check(hearRumor(*loaded, random)->id != last, "Rumor history lost across load");
    place(*loaded, "Attania");
    check(claimGuildReward(*loaded, "ironward"), "Loaded quest reward unavailable");
    check(saves::save(*loaded, second, error), "Cannot save claimed reward");
    loaded = saves::load(second, error);
    check(loaded && !claimGuildReward(*loaded, "ironward"), "Reload allowed duplicate reward");

    auto data = readFile(first);
    auto section = data.find("\"GUILD_STATE\"\n");
    check(section != std::string::npos, "No version 2 extension");
    auto legacy = data.substr(0, section);
    legacy.replace(0, std::string("RPGGAME 2").size(), "RPGGAME 1");
    writeFile(second, legacy);
    loaded = saves::load(second, error);
    check(loaded.has_value(), "Version 1 migration failed: " + error);
    check(loaded->guilds.guilds.empty() && loaded->guilds.cityLeads.empty() && loaded->guilds.rumors.empty(), "Legacy load granted guild progress");
    check(loaded->hero.stats.expe == session.hero.stats.expe && loaded->actionCounter == 2 &&
          loaded->hero.timeSystem.getCurrentTurn() == 5, "Legacy base state changed");
    auto invalid = data;
    auto id = invalid.find("\"ironward\"", section);
    invalid.replace(id, std::string("\"ironward\"").size(), "\"unknown_guild\"");
    writeFile(second, invalid);
    check(!saves::load(second, error), "Unknown saved guild accepted");
    invalid = data;
    auto stage = invalid.find("\"ironward\"\n1\n0\n1\n1\n", section);
    check(stage != std::string::npos, "Test stage fixture missing");
    invalid.replace(stage, std::string("\"ironward\"\n1\n0\n1\n1\n").size(), "\"ironward\"\n1\n2\n1\n1\n");
    writeFile(second, invalid);
    check(!saves::load(second, error), "Active quest after finished storyline accepted");
    invalid = data;
    auto rumorId = invalid.find("\"" + last + "\"", section);
    invalid.replace(rumorId, last.size() + 2, "\"unknown_rumor\"");
    writeFile(second, invalid);
    check(!saves::load(second, error), "Unknown saved rumor accepted");
    invalid = data.substr(0, data.size() - 10);
    writeFile(second, invalid);
    check(!saves::load(second, error), "Truncated guild save accepted");
}

void menus() {
    auto session = newSession();
    place(session, "Peaceful Town");
    {
        MenuIO io("4\n\n5\n");
        Tavern tavern(session.inventory, session.party, session);
        tavern.openTavern(session.hero, session.hero.timeSystem);
        check(io.text().find("Travel lead:") != std::string::npos, "Tavern did not show actionable rumor");
    }
    EnemyController enemies;
    CombatSystem combat;
    NPCGenerator generator;
    PlayerController stats(session.hero, session.inventory);
    TravelSystem travel(generator, session);
    const auto destination = *session.guilds.cityLeads.begin();
    {
        MenuIO io("3\n1\n\n3\n");
        travel.travel(session.hero, enemies, combat, session.inventory, stats, session.hero.timeSystem);
    }
    check(session.hero.currentLocation == destination && session.travel.discovered.count(destination), "Following rumor did not arrive in city");
    {
        MenuIO io("2\n0\n4\n3\n");
        mainMenu(session);
        check(io.text().find("Guild Hall") != std::string::npos && io.text().find("Magic Store") != std::string::npos, "City services missing");
        check(io.text().find("Your Party:") == std::string::npos, "City Explore started combat");
    }

    session = newSession();
    recordArrival(session, "Attania");
    {
        MenuIO io("1\n\n1\n\n2\n");
        openGuildHall(session);
    }
    check(activeGuildQuest(session.guilds, "ironward") != nullptr, "Guild hall did not accept first quest");
    // Fresh controllers are required after replacing the session state above.
    PlayerController questStats(session.hero, session.inventory);
    TravelSystem questTravel(generator, session);
    {
        MenuIO io("3\n2\n\n4\n1\n\n3\n");
        questTravel.travel(session.hero, enemies, combat, session.inventory, questStats, session.hero.timeSystem);
    }
    check(guildProgress(session.guilds, "ironward")->objectiveComplete, "Travel quest action did not retrieve supplies");
    {
        MenuIO io("3\n1\n3\n");
        questTravel.travel(session.hero, enemies, combat, session.inventory, questStats, session.hero.timeSystem);
    }
    {
        MenuIO io("1\n\n1\n\n2\n");
        openGuildHall(session);
    }
    // Mark the destination first to exercise explicit quest combat on safe travel.
    recordArrival(session, "Shadowed Grove");
    session.travel.marked.insert("Shadowed Grove");
    place(session, "Attania");
    {
        MenuIO io("3\n2\n4\n1\n4\n\n\n3\n");
        questTravel.travel(session.hero, enemies, combat, session.inventory, questStats, session.hero.timeSystem);
        check(io.text().find("VICTORY") != std::string::npos, "Quest fight did not run on marked route");
    }
    check(guildProgress(session.guilds, "ironward")->objectiveComplete, "Victory did not complete quest");
    auto money = copper(session);
    {
        MenuIO io("\n");
        performQuestAction(session, enemies, combat, generator);
    }
    check(copper(session) == money, "Completed quest farmed enemy drops");
    place(session, "Attania");
    {
        MenuIO io("1\n\n2\n");
        openGuildHall(session);
        check(io.text().find("Storyline complete") != std::string::npos, "Missing final completion screen");
    }
    check(guildProgress(session.guilds, "ironward")->completedQuests == 2, "Storyline not finished");
    {
        MenuIO io("\n");
        showQuestJournal(session);
        check(io.text().find("Storyline complete") != std::string::npos, "Journal lost completed guild");
    }
}

void combatOutcomes() {
    auto session = newSession();
    place(session, "Erden");
    joinGuild(session, "lantern"); acceptGuildQuest(session, "lantern");
    place(session, "Enchanted Forest"); completeGuildObjective(session, "lantern", true);
    place(session, "Erden"); claimGuildReward(session, "lantern"); acceptGuildQuest(session, "lantern");
    place(session, "Enchanted Library");
    EnemyController enemies;
    CombatSystem combat;
    NPCGenerator generator;
    float xp = session.hero.stats.expe;
    auto money = copper(session);
    {
        MenuIO io("1\n3\n\n");
        performQuestAction(session, enemies, combat, generator);
    }
    check(!guildProgress(session.guilds, "lantern")->objectiveComplete && session.hero.stats.expe == xp && copper(session) == money,
          "Retreat completed quest or paid rewards");
    session.hero.stats.hitpoints = 1;
    session.hero.stats.attack = session.hero.stats.magicAttack = 0;
    session.hero.stats.dodgeRate = 0;
    session.hero.stats.armor = session.hero.stats.magicArmor = 0;
    session.hero.timeSystem.setState(3, TimeSystem::TimePeriod::Night, 0);
    {
        MenuIO io("1\n1\n\n\n");
        performQuestAction(session, enemies, combat, generator);
    }
    check(session.hero.stats.hitpoints <= 0 && !guildProgress(session.guilds, "lantern")->objectiveComplete,
          "Defeat completed quest or revived player at day boundary");
    check(copper(session) == money && session.hero.stats.expe == xp, "Defeat granted rewards");
    session.hero.stats.hitpoints = 10000;
    NPC helper{};
    helper.name = "Quest Companion";
    helper.stats = session.hero.stats;
    helper.stats.attack = 10000;
    helper.stats.critRate = 0;
    session.party.push_back(helper);
    {
        MenuIO io("1\n1\n\n\n");
        performQuestAction(session, enemies, combat, generator);
        check(io.text().find("Quest Companion attacked Bonewalker") != std::string::npos, "Companion absent from quest fight");
    }
    check(guildProgress(session.guilds, "lantern")->objectiveComplete, "Victory after retry did not complete quest");
}
}

int main(int argc, char** argv) {
    try {
        check(argc >= 2, "Pass test group");
        std::string group = argv[1];
        if (group == "progression") progression();
        else if (group == "rumors") rumors();
        else if (group == "save") {
            check(argc == 3, "Pass test directory");
            savesTest(argv[2]);
        } else if (group == "menus") { menus(); combatOutcomes(); }
        else throw std::runtime_error("Unknown test group");
        std::cout << "Passed guild " << group << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
