#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>

#include "app/session.hpp"
#include "combat/combat.hpp"
#include "data/class.hpp"
#include "data/race.hpp"
#include "systems/guild.hpp"
#include "systems/save.hpp"
#include "world/comissions.hpp"
#include "world/encounters.hpp"
#include "world/guild_quest.hpp"
#include "world/quest.hpp"
#include "world/quest_system.hpp"
#include "world/sidequests.hpp"
#include "world/travel.hpp"

namespace {
void check(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

GameSession newSession() {
    PlayerClassCollection classes;
    PlayerRaceDatabase races;
    Player hero("Job Tester", {races.templates[0].name, {races.templates[0].lore.description}}, classes.getClassTemplate("Ranger"));
    hero.applyRaceBonus(races.templates[0].statBonus);
    hero.stats.hitpoints = hero.stats.maxHitpoints = 10000;
    hero.stats.attack = 10000;
    hero.progression.experienceRequired = 1000000;
    return GameSession(hero);
}

void place(GameSession& session, const std::string& name) {
    const auto area = currentArea(name);
    check(area.name == name, "Unknown test location");
    session.hero.currentLocation = area.name;
    session.hero.currentLocationType = area.type;
}

long long money(const GameSession& session) {
    const auto& e = session.hero.economy;
    return ((static_cast<long long>(e.platinum) * 100 + e.gold) * 100 + e.silver) * 100 + e.copper;
}

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

void catalog() {
    check(sideQuests().size() == 20 && commissions().size() == 20, "Expected 40 new quests, split 20/20");
    check(mainQuests().empty(), "Main content should remain empty");
    check(allQuests().size() == 50, "Lost existing guild quests");
    std::set<std::string> ids, titles;
    EnemyDatabase enemies;
    int general = 0, restricted = 0, prerequisite = 0, retrieval = 0, fight = 0;
    for (const auto& quest : allQuests()) {
        check(ids.insert(quest.id).second && titles.insert(quest.title).second, "Duplicate quest ID/title");
        check(!quest.briefing.empty() && !quest.completionText.empty() && quest.rewardSilver > 0 && quest.rewardExperience > 0 &&
              quest.minimumLevel > 0, "Incomplete quest content");
        check(currentArea(quest.location).name == quest.location, "Invalid objective location");
        if (!quest.enemyName.empty()) {
            check(std::any_of(enemies.templates.begin(), enemies.templates.end(), [&](const auto& e) { return e.name == quest.enemyName; }), "Unknown quest enemy");
            ++fight;
        } else ++retrieval;
        if (!quest.offeringGuild.empty()) check(guildDatabase().find(quest.offeringGuild), "Unknown offering guild");
        if (!quest.prerequisite.empty()) {
            const auto* required = findQuest(quest.prerequisite);
            check(required && required->category == QuestCategory::Guild, "Unknown guild prerequisite");
            ++prerequisite;
        }
        if (quest.category == QuestCategory::Side || quest.category == QuestCategory::Commission) {
            if (quest.offeringGuild.empty()) ++general;
            else ++restricted;
        }
    }
    check(general == 20 && restricted == 20 && prerequisite >= 10 && retrieval && fight, "Missing requested content/access variety");
    for (const auto& q : sideQuests()) check(q.category == QuestCategory::Side, "Wrong side category");
    for (const auto& q : commissions()) check(q.category == QuestCategory::Commission, "Wrong commission category");
}

void requirements() {
    auto session = newSession();
    check(boardQuests(session, QuestCategory::Side).empty(), "Board outside guild hall");
    check(!acceptQuest(session, "side_unsent_letter"), "Accepted outside guild hall");
    for (const auto& guild : guildDatabase().getGuilds()) {
        place(session, guild.city);
        for (auto category : {QuestCategory::Side, QuestCategory::Commission}) {
            auto board = boardQuests(session, category);
            check(board.size() == 12, "Expected 10 general and 2 local jobs per board");
            for (const auto* q : board) check(q->offeringGuild.empty() || q->offeringGuild == guild.id, "Other guild's jobs leaked");
        }
        check(questLockReason(session, *findQuest("side_unsent_letter")).empty(), "General level-1 job requires membership");
    }
    place(session, "Attania");
    discoverBoardQuests(session, QuestCategory::Side);
    check(session.quests.jobs.size() == 12, "Board did not discover visible jobs");
    check(!acceptQuest(session, "side_keepsake"), "Level requirement bypassed");
    session.hero.stats.level = 2;
    check(acceptQuest(session, "side_keepsake"), "Level-only job unavailable");
    check(!acceptQuest(session, "side_warning_bell"), "Guild-only job accepted without joining");
    check(joinGuild(session, "ironward"), "Cannot join Ironward");
    check(acceptQuest(session, "side_warning_bell"), "Guild-only job failed after joining");
    check(!acceptQuest(session, "side_warden_watch"), "Prerequisite bypassed");
    check(acceptGuildQuest(session, "ironward"), "Cannot accept prerequisite");
    place(session, "Barren Plains");
    check(completeGuildObjective(session, "ironward", true), "Cannot complete prerequisite objective");
    place(session, "Attania");
    check(!acceptQuest(session, "side_warden_watch"), "Objective completion incorrectly counted as turn-in");
    check(claimGuildReward(session, "ironward"), "Cannot turn in prerequisite");
    session.hero.stats.level = 1;
    check(!acceptQuest(session, "side_warden_watch"), "Combined level/prerequisite gate bypassed");
    session.hero.stats.level = 2;
    check(acceptQuest(session, "side_warden_watch"), "Combined requirements not satisfied");
    place(session, "Erden");
    check(!acceptQuest(session, "commission_watch_rations"), "Local job accepted at wrong guild");
    check(!acceptQuest(session, "side_keepsake"), "Same active quest accepted at another guild");
    place(session, "Barren Plains");
    check(completeQuestObjective(session, "side_warning_bell", true), "Cannot complete local job");
    check(!questProgress(session, "side_warden_watch").objectiveComplete, "One action completed two quests at same location");
    place(session, "Erden");
    check(!claimQuestReward(session, "side_warning_bell"), "Local quest turned in at wrong guild");
    place(session, "Attania");
    check(claimQuestReward(session, "side_warning_bell"), "Cannot turn in local job");
    validateGuildState(session);
    validateQuestState(session);
}

void lifecycle() {
    auto session = newSession();
    place(session, "Attania");
    check(acceptQuest(session, "side_unsent_letter"), "General side acceptance failed");
    check(!acceptQuest(session, "side_unsent_letter"), "Duplicate acceptance");
    check(travelLeads(session).count("Ghost Town") && session.guilds.cityLeads.count("Attania"), "Missing objective/return leads");
    check(session.travel.discovered.empty(), "Acceptance counted as a visit");
    check(!completeQuestObjective(session, "side_unsent_letter", true), "Wrong-location completion");
    place(session, "Ghost Town");
    check(!completeQuestObjective(session, "side_unsent_letter", false), "Retreat completed side quest");
    session.hero.stats.hitpoints = 0;
    check(!completeQuestObjective(session, "side_unsent_letter", true), "Dead hero completed side quest");
    session.hero.stats.hitpoints = 10000;
    check(completeQuestObjective(session, "side_unsent_letter", true), "Side objective failed");
    check(!completeQuestObjective(session, "side_unsent_letter", true), "Repeated ready objective");
    check(!travelLeads(session).count("Ghost Town"), "Ready objective kept unfinished lead");
    place(session, "Erden");
    session.hero.stats.hitpoints = 0;
    check(!claimQuestReward(session, "side_unsent_letter"), "Dead hero claimed reward");
    session.hero.stats.hitpoints = 10000;
    float xp = session.hero.stats.expe;
    check(claimQuestReward(session, "side_unsent_letter"), "General quest could not turn in at another guild");
    check(money(session) == 4000 && session.hero.stats.expe == xp + 25, "Wrong side reward");
    check(!claimQuestReward(session, "side_unsent_letter") && !acceptQuest(session, "side_unsent_letter"), "One-time quest repeated");

    for (int run = 0; run < 3; ++run) {
        place(session, run % 2 ? "Attania" : "Erden");
        check(acceptQuest(session, "commission_supplies"), "Could not accept repeat commission");
        auto progress = questProgress(session, "commission_supplies");
        check(progress.active && !progress.objectiveComplete && progress.completions == run, "Repeat run did not reset objective");
        check(!claimQuestReward(session, "commission_supplies"), "Reacceptance replayed reward");
        place(session, "Barren Plains");
        check(completeQuestObjective(session, "commission_supplies", true), "Repeat objective failed");
        auto before = money(session);
        place(session, "Velmora");
        check(claimQuestReward(session, "commission_supplies"), "General commission turn-in failed");
        check(money(session) == before + 1500, "Wrong commission reward");
        check(!claimQuestReward(session, "commission_supplies") && money(session) == before + 1500, "Double commission payout");
        progress = questProgress(session, "commission_supplies");
        check(!progress.active && !progress.objectiveComplete && progress.completions == run + 1, "Commission auto-accepted or lost count");
    }
    validateQuestState(session);
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}
void writeFile(const std::filesystem::path& path, const std::string& data) { std::ofstream(path, std::ios::binary) << data; }

void persistence(const std::filesystem::path& directory) {
    auto file = directory / "quests-v3.dat", copy = directory / "quests-copy.dat";
    auto session = newSession();
    place(session, "Attania");
    joinGuild(session, "ironward");
    acceptGuildQuest(session, "ironward");
    discoverBoardQuests(session, QuestCategory::Side);
    std::string error;
    auto roundtrip = [&] {
        check(saves::save(session, file, error), "Save failed: " + error);
        auto loaded = saves::load(file, error);
        check(loaded.has_value(), "Load failed: " + error);
        check(saves::save(*loaded, copy, error), "Resave failed");
        check(readFile(file) == readFile(copy), "Quest state round trip changed data");
        session = *loaded;
    };
    roundtrip(); // Discovered but locked/unaccepted entries are valid.
    check(acceptQuest(session, "side_unsent_letter"), "Side accept failed");
    check(acceptQuest(session, "commission_supplies"), "Commission accept failed");
    roundtrip();
    for (int run = 0; run < 2; ++run) {
        place(session, "Barren Plains");
        completeQuestObjective(session, "commission_supplies", true);
        roundtrip();
        check(questProgress(session, "commission_supplies").objectiveComplete, "Lost ready state");
        place(session, "Erden");
        check(claimQuestReward(session, "commission_supplies"), "Loaded commission could not pay");
        roundtrip();
        check(!claimQuestReward(session, "commission_supplies"), "Load replayed payout");
        check(questProgress(session, "commission_supplies").completions == run + 1, "Lost completion count");
        if (run == 0) { check(acceptQuest(session, "commission_supplies"), "Loaded commission cannot repeat"); roundtrip(); }
    }
    place(session, "Ghost Town"); completeQuestObjective(session, "side_unsent_letter", true);
    place(session, "Trisselie"); claimQuestReward(session, "side_unsent_letter");
    roundtrip();
    check(!acceptQuest(session, "side_unsent_letter"), "Loaded side quest became repeatable");
    const auto data = readFile(file);
    for (int version : {1, 2}) {
        auto marker = data.find(version == 1 ? "\"GUILD_STATE\"\n" : "\"QUEST_STATE\"\n");
        check(marker != std::string::npos, "Migration marker missing");
        auto legacy = data.substr(0, marker);
        legacy.replace(0, 9, "RPGGAME " + std::to_string(version));
        writeFile(copy, legacy);
        auto loaded = saves::load(copy, error);
        check(loaded.has_value(), "Legacy migration failed: " + error);
        check(loaded->quests.jobs.empty(), "Legacy migration granted new jobs");
        check(loaded->hero.stats.expe == session.hero.stats.expe && money(*loaded) == money(session), "Migration replayed rewards");
        if (version == 2) check(guildProgress(loaded->guilds, "ironward")->active, "Version 2 lost active guild quest");
    }
    auto invalid = data;
    auto section = invalid.find("\"QUEST_STATE\"\n");
    auto id = invalid.find("\"side_unsent_letter\"", section);
    invalid.replace(id, std::string("\"side_unsent_letter\"").size(), "\"bad_quest\"");
    writeFile(copy, invalid);
    check(!saves::load(copy, error), "Unknown quest accepted");
    invalid = data;
    auto state = invalid.find("\"side_unsent_letter\"\n0\n0\n1\n", section);
    check(state != std::string::npos, "Side-state test fixture missing");
    invalid.replace(state, std::string("\"side_unsent_letter\"\n0\n0\n1\n").size(), "\"side_unsent_letter\"\n0\n0\n2\n");
    writeFile(copy, invalid);
    check(!saves::load(copy, error), "Repeated one-time quest accepted");
    invalid = data;
    state = invalid.find("\"commission_supplies\"\n0\n0\n2\n\"lantern\"", section);
    check(state != std::string::npos, "Commission-state test fixture missing");
    invalid.replace(state, std::string("\"commission_supplies\"\n0\n0\n2\n\"lantern\"").size(),
                    "\"commission_supplies\"\n0\n1\n2\n\"lantern\"");
    writeFile(copy, invalid);
    check(!saves::load(copy, error), "Inactive completed objective accepted");
    auto corrupt = session;
    for (auto& job : corrupt.quests.jobs) if (job.questId == "side_warden_watch") {
        job.active = true;
        job.acceptingGuild = "ironward";
    }
    check(!saves::save(corrupt, file, error), "Missing level requirement accepted");
    corrupt.hero.stats.level = 2;
    check(!saves::save(corrupt, file, error), "Missing guild prerequisite accepted");
    check(readFile(file) == data, "Invalid quest state destroyed existing slot");
}

void menus() {
    auto session = newSession();
    place(session, "Attania");
    {
        MenuIO io("3\n1\n1\n\n0\n2\n");
        openGuildHall(session);
        check(io.text().find("Locked: requires level 2") != std::string::npos, "Board omitted lock reasons");
    }
    check(questProgress(session, "side_unsent_letter").active, "Guild board could not accept side quest");
    {
        MenuIO io("4\n1\n1\n\n0\n2\n");
        openGuildHall(session);
    }
    check(questProgress(session, "commission_supplies").active, "Guild board could not accept commission");
    joinGuild(session, "ironward"); acceptGuildQuest(session, "ironward");
    place(session, "Barren Plains");
    auto ids = questsAtLocation(session);
    check(ids.size() == 2 && ids[0] == "ironward_supplies" && ids[1] == "commission_supplies", "Shared quest action lost a category");
    EnemyController enemies;
    CombatSystem combat;
    NPCGenerator generator;
    {
        MenuIO io("2\n\n");
        performQuestAction(session, enemies, combat, generator);
        check(io.text().find("[Guild quests]") != std::string::npos && io.text().find("[Commissions]") != std::string::npos, "Action menu omitted categories");
    }
    check(questProgress(session, "commission_supplies").objectiveComplete && !guildProgress(session.guilds, "ironward")->objectiveComplete,
          "One action credited multiple objectives");
    check(session.hero.timeSystem.getCurrentTurn() == 1, "Objective time cost incorrect");
    place(session, "Erden");
    {
        MenuIO io("4\n1\n1\n\n0\n2\n");
        openGuildHall(session);
    }
    check(questProgress(session, "commission_supplies").completions == 1, "Board failed to turn in general job elsewhere");
    {
        MenuIO io("\n");
        showQuestJournal(session);
        check(io.text().find("No main quests available yet") != std::string::npos && io.text().find("Runs completed: 1") != std::string::npos,
              "Journal missing main placeholder or repeat count");
    }
    // Finishing a guild storyline must not close access to its boards.
    place(session, "Attania");
    auto& guild = discoverGuild(session.guilds, "ironward");
    guild.joined = true; guild.completedQuests = 2; guild.active = guild.objectiveComplete = false;
    {
        MenuIO io("3\n0\n4\n0\n2\n");
        openGuildHall(session);
        check(io.text().find("Storyline complete") != std::string::npos && io.text().find("=== Commissions ===") != std::string::npos,
              "Finished guild lost its boards");
    }

    session = newSession();
    place(session, "Attania"); acceptQuest(session, "commission_rats");
    place(session, "Ghost Town");
    {
        MenuIO io("1\n3\n\n");
        performQuestAction(session, enemies, combat, generator);
    }
    check(!questProgress(session, "commission_rats").objectiveComplete && money(session) == 0, "Retreat granted commission completion/rewards");
    session.hero.stats.hitpoints = 1;
    session.hero.stats.attack = session.hero.stats.magicAttack = 0;
    session.hero.stats.armor = session.hero.stats.magicArmor = session.hero.stats.dodgeRate = 0;
    {
        MenuIO io("1\n1\n\n\n");
        performQuestAction(session, enemies, combat, generator);
    }
    check(session.hero.stats.hitpoints <= 0 && !questProgress(session, "commission_rats").objectiveComplete, "Defeat completed commission");
    session.hero.stats.hitpoints = 10000;
    session.hero.stats.attack = 10000;
    {
        MenuIO io("1\n4\n\n\n");
        performQuestAction(session, enemies, combat, generator);
    }
    check(questProgress(session, "commission_rats").objectiveComplete, "Retry victory failed");
    auto reward = money(session);
    {
        MenuIO io("\n");
        performQuestAction(session, enemies, combat, generator);
    }
    check(money(session) == reward, "Ready commission farmed combat loot");
}
}

int main(int argc, char** argv) {
    try {
        check(argc >= 2, "Pass test group");
        std::string group = argv[1];
        if (group == "catalog") catalog();
        else if (group == "rules") requirements();
        else if (group == "lifecycle") lifecycle();
        else if (group == "save") { check(argc == 3, "Pass scratch directory"); persistence(argv[2]); }
        else if (group == "menus") menus();
        else throw std::runtime_error("Unknown test group");
        std::cout << "Passed quest " << group << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
