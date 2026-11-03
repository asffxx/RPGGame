#include "conditions.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>

#include "app/session.hpp"
#include "catalogs/class.hpp"
#include "catalogs/content_ids.hpp"
#include "catalogs/guild.hpp"
#include "catalogs/race.hpp"
#include "world/crafting.hpp"
#include "world/quest_system.hpp"

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
void fields(const datafile::Value& value, const std::set<std::string>& allowed) {
    for (const auto& field : value.object())
        require(allowed.count(field.first), "unknown field: " + field.first);
}
std::string text(const datafile::Value& value, const std::string& key) {
    auto result = value.at(key).string();
    require(!result.empty() && result.size() <= 4096, key + ": expected nonempty text (max 4096 bytes)");
    return result;
}
std::string optionalText(const datafile::Value& value, const std::string& key) {
    return value.has(key) ? value.at(key).string() : "";
}
std::string identifier(const datafile::Value& value, const std::string& key) {
    auto result = text(value, key);
    require(ids::valid(result), key + ": invalid id " + result);
    return result;
}
bool recorded(const GameSession& s, const std::string& dialogue, const std::string& choice) {
    return std::any_of(s.story.choices.begin(), s.story.choices.end(), [&](const ChoiceRecord& r) {
        return r.dialogueId == dialogue && r.choiceId == choice;
    });
}
bool known(const GameSession& s, const std::string& id) {
    return std::find(s.story.knownRecipes.begin(), s.story.knownRecipes.end(), id) != s.story.knownRecipes.end();
}

const std::set<std::string> counters = {
    "level", "quests_completed", "recipes_known", "guilds_joined",
    "cities_discovered", "choices_recorded", "max_standing",
};

int counterValue(const GameSession& s, const std::string& id) {
    if (id == "level") return s.hero.stats.level;
    if (id == "recipes_known") return static_cast<int>(s.story.knownRecipes.size());
    if (id == "choices_recorded") return static_cast<int>(s.story.choices.size());
    if (id == "guilds_joined") {
        return static_cast<int>(std::count_if(s.guilds.guilds.begin(), s.guilds.guilds.end(),
            [](const GuildProgress& g) { return g.joined; }));
    }
    if (id == "cities_discovered") {
        std::set<std::string> cities;
        for (const auto& guild : guildDatabase().getGuilds())
            if (cityVisited(s, guild.city)) cities.insert(guild.city);
        return static_cast<int>(cities.size());
    }
    if (id == "max_standing") {
        int value = s.story.factions.empty() ? 0 : minStanding;
        for (const auto& faction : s.story.factions) value = std::max(value, faction.standing);
        return value;
    }
    if (id == "quests_completed") {
        int value = 0;
        for (const auto& quest : allQuests())
            if (questProgress(s, quest.id).completions > 0) ++value;
        return value;
    }
    return 0;
}

std::string counterLabel(const std::string& id) {
    static const std::map<std::string, std::string> labels = {
        {"level", "character levels"},
        {"quests_completed", "completed quests"},
        {"recipes_known", "known recipes"},
        {"guilds_joined", "joined guilds"},
        {"cities_discovered", "discovered cities"},
        {"choices_recorded", "recorded choices"},
        {"max_standing", "standing"},
    };
    auto found = labels.find(id);
    return found == labels.end() ? id : found->second;
}
}

bool cityVisited(const GameSession& s, const std::string& city) {
    return s.hero.currentLocation == city || s.travel.discovered.count(city) > 0;
}

ConditionRule parseRule(const datafile::Value& value, bool effect,
                        const PlayerInventory& inventory) {
    static const std::set<std::string> allowedFields = {"type", "id", "state", "value"};
    fields(value, allowedFields);
    ConditionRule r;
    r.type = text(value, "type");
    r.id = identifier(value, "id");
    r.state = optionalText(value, "state");
    r.value = value.has("value") ? value.at("value").integer() : 0;
    static const std::set<std::string> effects = {"flag", "item", "quest", "standing", "recipe"};
    static const std::set<std::string> conditions = {"flag", "item", "quest", "standing", "recipe", "choice", "class", "race", "count"};
    require((effect ? effects : conditions).count(r.type), "unknown rule type: " + r.type);
    if (r.type == "flag" || r.type == "standing" || r.type == "item" || r.type == "count")
        require(value.has("value"), "rule requires value: " + r.type);
    if (r.type == "flag") {
        if (!effect) require(r.state == "eq" || r.state == "ne" || r.state == "gte", "flag requires eq/ne/gte state");
    } else if (r.type == "standing") {
        require(guildDatabase().find(r.id), "unknown guild: " + r.id);
        require(r.value >= -100 && r.value <= 100, "standing value outside -100..100");
    } else if (r.type == "item") {
        PlayerInventory::ItemType type;
        size_t index;
        require(inventory.resolveItemId(r.id, type, index), "unknown item: " + r.id);
        require(r.value > 0 && r.value <= 1000, "item quantity outside 1..1000");
    } else if (r.type == "quest") {
        require(findQuest(r.id), "unknown quest: " + r.id);
        if (!effect) require(r.state == "available" || r.state == "active" || r.state == "objective" || r.state == "completed", "invalid quest state");
    } else if (r.type == "recipe") {
        require(craftingDatabase().find(r.id), "unknown recipe: " + r.id);
        if (!effect) require(r.state == "known" || r.state == "unknown", "invalid recipe state");
    } else if (r.type == "class") {
        require(PlayerClassCollection().findById(r.id), "unknown class: " + r.id);
    } else if (r.type == "race") {
        require(PlayerRaceDatabase().findById(r.id), "unknown race: " + r.id);
    } else if (r.type == "choice") {
        require(r.state == "recorded" || r.state == "unrecorded", "invalid choice state");
    } else if (r.type == "count") {
        require(counters.count(r.id), "unknown counter: " + r.id);
        require(r.state == "gte", "count requires gte state");
        require(r.value >= -100000 && r.value <= 100000, "count value out of range");
    }
    const bool needsState = !effect && (r.type == "flag" || r.type == "quest" || r.type == "recipe" || r.type == "choice" || r.type == "count");
    require(needsState || !value.has("state"), "unexpected state on " + r.type);
    const bool needsValue = r.type == "flag" || r.type == "standing" || r.type == "item" || r.type == "count";
    require(needsValue || !value.has("value"), "unexpected value on " + r.type);
    return r;
}

bool conditionPasses(const GameSession& s, const std::string& dialogueScope,
                     const ConditionRule& r) {
    if (r.type == "flag") {
        const auto* f = s.story.flag(r.id);
        int v = f ? f->value : 0;
        return r.state == "eq" ? v == r.value : r.state == "ne" ? v != r.value : v >= r.value;
    }
    if (r.type == "standing") return s.story.standing(r.id) >= r.value;
    if (r.type == "item") {
        PlayerInventory::ItemType type; size_t index;
        return s.inventory.resolveItemId(r.id, type, index) && s.inventory.countItem(type, index) >= r.value;
    }
    if (r.type == "recipe") return known(s, r.id) == (r.state == "known");
    if (r.type == "choice") return recorded(s, dialogueScope, r.id) == (r.state == "recorded");
    if (r.type == "class") return s.hero.playerClass.id == r.id;
    if (r.type == "race") {
        PlayerRaceDatabase races;
        const auto* entry = races.findById(r.id);
        return entry && entry->name == s.hero.race.name;
    }
    if (r.type == "quest") {
        auto p = questProgress(s, r.id);
        if (r.state == "active") return p.active;
        if (r.state == "objective") return p.objectiveComplete;
        if (r.state == "completed") return p.completions > 0;
        auto staged = s;
        return acceptQuest(staged, r.id);
    }
    if (r.type == "count") return counterValue(s, r.id) >= r.value;
    return false;
}

std::string conditionFailure(const GameSession& s, const std::string& dialogueScope,
                             const ConditionRule& r) {
    if (conditionPasses(s, dialogueScope, r)) return "";
    if (r.type == "flag") return "A different local decision or earlier conversation is required.";
    if (r.type == "recipe") return r.state == "unknown" ? "Recipe already known." : "Learn the recipe first.";
    if (r.type == "item") return "Requires " + std::to_string(r.value) + " of " + r.id + ".";
    if (r.type == "standing") return "Requires standing " + std::to_string(r.value) + " with " + r.id + ".";
    if (r.type == "count") return "Requires at least " + std::to_string(r.value) + " " + counterLabel(r.id) + ".";
    return "Requires " + r.type + ": " + r.id + (r.state.empty() ? "" : " (" + r.state + ")") + ".";
}
