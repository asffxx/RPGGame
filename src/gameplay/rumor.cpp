#include "rumor.hpp"

#include <algorithm>
#include "app/display.hpp"

std::vector<const RumorDefinition*> eligibleRumors(const GameSession& session) {
    if (session.hero.currentLocationType != PeacefulTown && session.hero.currentLocationType != PeacefulCity) return {};
    const auto& city = session.hero.currentLocation;
    std::string last;
    for (const auto& entry : session.guilds.rumors) if (entry.tavern == city) last = entry.rumorId;
    std::vector<const RumorDefinition*> pool;
    for (const auto& rumor : rumorDatabase().getRumors()) {
        if ((!rumor.destination.empty() && rumor.destination != city) || rumor.city == city)
            pool.push_back(&rumor);
    }
    if (pool.size() > 1) {
        pool.erase(std::remove_if(pool.begin(), pool.end(), [&](const auto* rumor) { return rumor->id == last; }), pool.end());
    }
    std::vector<const RumorDefinition*> newCities;
    for (const auto* rumor : pool) {
        if (!rumor->destination.empty() && !session.guilds.cityLeads.count(rumor->destination) &&
            !session.travel.discovered.count(rumor->destination)) newCities.push_back(rumor);
    }
    return newCities.empty() ? pool : newCities;
}

const RumorDefinition* hearRumor(GameSession& session, std::mt19937& random) {
    auto pool = eligibleRumors(session);
    if (pool.empty()) return nullptr;
    std::uniform_int_distribution<size_t> pick(0, pool.size() - 1);
    const auto* rumor = pool[pick(random)];
    if (!rumor->destination.empty()) session.guilds.cityLeads.insert(rumor->destination);
    auto& history = session.guilds.rumors;
    auto found = std::find_if(history.begin(), history.end(), [&](const auto& h) { return h.tavern == session.hero.currentLocation; });
    if (found == history.end()) history.push_back({session.hero.currentLocation, rumor->id});
    else found->rumorId = rumor->id;
    return rumor;
}

void listenToRumors(GameSession& session) {
    static std::mt19937 random(std::random_device{}());
    const auto* rumor = hearRumor(session, random);
    if (!rumor) { display::message("No rumors are circulating here."); return; }
    std::string text = rumor->text;
    if (!rumor->destination.empty()) text += "\nTravel lead: " + rumor->destination + ". Choose Travel -> Follow a lead.";
    display::message(text);
}
