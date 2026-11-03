#include "dialogue.hpp"

#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include "app/session.hpp"
#include "app/display.hpp"
#include "catalogs/class.hpp"
#include "catalogs/race.hpp"
#include "catalogs/guild.hpp"
#include "catalogs/content_ids.hpp"
#include "catalogs/content_paths.hpp"
#include "core/ui.hpp"
#include "gameplay/datafile.hpp"
#include "world/crafting.hpp"
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
std::string optionalText(const Value& value, const std::string& key) {
    return value.has(key) ? value.at(key).string() : "";
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
bool recorded(const GameSession& s, const std::string& dialogue, const std::string& choice) {
    return std::any_of(s.story.choices.begin(), s.story.choices.end(), [&](const ChoiceRecord& r) {
        return r.dialogueId == dialogue && r.choiceId == choice;
    });
}
bool known(const GameSession& s, const std::string& id) {
    return std::find(s.story.knownRecipes.begin(), s.story.knownRecipes.end(), id) != s.story.knownRecipes.end();
}

DialogueRule rule(const Value& value, bool effect, const PlayerInventory& inventory) {
    fields(value, {"type", "id", "state", "value"});
    DialogueRule r;
    r.type = text(value, "type");
    r.id = identifier(value, "id");
    r.state = optionalText(value, "state");
    r.value = value.has("value") ? value.at("value").integer() : 0;
    const std::set<std::string> effects = {"flag", "item", "quest", "standing", "recipe"};
    const std::set<std::string> conditions = {"flag", "item", "quest", "standing", "recipe", "choice", "class", "race"};
    require((effect ? effects : conditions).count(r.type), "unknown rule type: " + r.type);
    if (r.type == "flag" || r.type == "standing" || r.type == "item")
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
    }
    const bool needsState = !effect && (r.type == "flag" || r.type == "quest" || r.type == "recipe" || r.type == "choice");
    require(needsState || !value.has("state"), "unexpected state on " + r.type);
    const bool needsValue = r.type == "flag" || r.type == "standing" || r.type == "item";
    require(needsValue || !value.has("value"), "unexpected value on " + r.type);
    return r;
}

int input(size_t maximum) {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.find_first_not_of(" \t\r") == std::string::npos) continue;
        std::istringstream stream(line);
        int number;
        char extra;
        if (stream >> number && !(stream >> extra) && number >= 0 && static_cast<size_t>(number) <= maximum) return number;
        display::message("Enter one of the listed numbers.");
    }
    return 0;
}
}

const DialogueNode* DialogueResident::node(const std::string& id) const {
    for (const auto& n : nodes) if (n.id == id) return &n;
    return nullptr;
}
const DialogueResident* DialogueDatabase::find(const std::string& id) const {
    for (const auto& r : residents_) if (r.id == id) return &r;
    return nullptr;
}

bool DialogueDatabase::load(const std::filesystem::path& root, std::string& error) {
    loaded_ = false;
    residents_.clear();
    error_.clear();
    error.clear();
    std::string where = (root / "characters/dialogue_npcs.json").string();
    try {
        const auto roster = read(where);
        fields(roster, {"residents"});
        require(roster.at("residents").size() > 0 && roster.at("residents").size() <= 100, "expected 1..100 residents");
        PlayerInventory inventory;
        for (const auto& quest : allQuests()) {
            std::set<std::string> rewardIds;
            for (const auto& reward : quest.itemRewards) {
                PlayerInventory::ItemType type;
                size_t index;
                require(reward.quantity >= 1 && reward.quantity <= 1000 &&
                        rewardIds.insert(reward.item).second && inventory.resolveItemId(reward.item, type, index),
                        "invalid item reward on quest " + quest.id);
            }
        }
        std::set<std::string> residents;
        for (const auto& entry : roster.at("residents").array()) {
            where = (root / "characters/dialogue_npcs.json").string();
            fields(entry, {"id", "name", "city", "role"});
            DialogueResident npc;
            npc.id = identifier(entry, "id");
            require(residents.insert(npc.id).second, "duplicate resident id: " + npc.id);
            npc.name = text(entry, "name");
            npc.city = text(entry, "city");
            npc.role = text(entry, "role");
            require(guildDatabase().inCity(npc.city), "unknown city: " + npc.city);
            where = (root / "narrative/dialogues" / (npc.id + ".json")).string();
            const auto graph = read(where);
            fields(graph, {"entry", "nodes"});
            npc.entry = identifier(graph, "entry");
            require(graph.at("nodes").size() > 0 && graph.at("nodes").size() <= 200, "expected 1..200 nodes");
            std::set<std::string> nodes, choices;
            for (const auto& value : graph.at("nodes").array()) {
                fields(value, {"id", "text", "choices"});
                DialogueNode node;
                node.id = identifier(value, "id");
                node.text = text(value, "text");
                require(nodes.insert(node.id).second, "duplicate node: " + node.id);
                require(value.at("choices").size() <= 40, "too many choices");
                for (const auto& item : value.at("choices").array()) {
                    fields(item, {"id", "text", "next", "result", "once", "conditions", "effects"});
                    DialogueChoice choice;
                    choice.id = identifier(item, "id");
                    choice.text = text(item, "text");
                    choice.next = optionalText(item, "next");
                    choice.result = optionalText(item, "result");
                    require(choice.result.size() <= 4096, "consequence text too long");
                    choice.once = item.has("once") && item.at("once").boolean();
                    require(choices.insert(choice.id).second, "duplicate choice: " + choice.id);
                    if (item.has("conditions")) for (const auto& v : item.at("conditions").array()) choice.conditions.push_back(rule(v, false, inventory));
                    if (item.has("effects")) for (const auto& v : item.at("effects").array()) choice.effects.push_back(rule(v, true, inventory));
                    require(choice.effects.size() <= 32 && choice.conditions.size() <= 32, "too many rules");
                    require(choice.effects.empty() || choice.once, "effectful choices must be once-only");
                    require(!choice.once || !choice.result.empty(), "once-only choices require consequence text");
                    require(choice.once || choice.result.empty(), "recorded choices must be once-only");
                    node.choices.push_back(std::move(choice));
                }
                npc.nodes.push_back(std::move(node));
            }
            require(npc.node(npc.entry), "missing entry node");
            for (const auto& n : npc.nodes) for (const auto& c : n.choices) {
                require(c.next.empty() || npc.node(c.next), "unknown next node: " + c.next);
                for (const auto& r : c.conditions) if (r.type == "choice") require(choices.count(r.id), "unknown choice reference: " + r.id);
            }
            std::set<std::string> reached;
            std::vector<std::string> pending{npc.entry};
            while (!pending.empty()) {
                auto id = pending.back(); pending.pop_back();
                if (!reached.insert(id).second) continue;
                for (const auto& c : npc.node(id)->choices) if (!c.next.empty()) pending.push_back(c.next);
            }
            require(reached.size() == npc.nodes.size(), "unreachable dialogue node");
            residents_.push_back(std::move(npc));
        }
        loaded_ = true;
        return true;
    } catch (const std::exception& e) {
        error_ = where + ": " + e.what();
        error = error_;
        residents_.clear();
        return false;
    }
}

const DialogueDatabase& dialogueDatabase() {
    static const DialogueDatabase database = [] {
        DialogueDatabase result;
        std::string error;
        result.load(paths::dataDir(), error);
        return result;
    }();
    return database;
}

std::string dialogueLockReason(const GameSession& s, const DialogueResident& npc, const DialogueChoice& c) {
    if (s.hero.currentLocationType != PeacefulCity || s.hero.currentLocation != npc.city) return "Visit " + npc.city + ".";
    if (s.hero.stats.hitpoints <= 0) return "You must be alive to speak.";
    if (c.once && recorded(s, npc.id, c.id)) return "Already decided.";
    for (const auto& r : c.conditions) {
        bool pass = false;
        if (r.type == "flag") {
            const auto* f = s.story.flag(r.id);
            int v = f ? f->value : 0;
            pass = r.state == "eq" ? v == r.value : r.state == "ne" ? v != r.value : v >= r.value;
        } else if (r.type == "standing") pass = s.story.standing(r.id) >= r.value;
        else if (r.type == "item") {
            PlayerInventory::ItemType type; size_t index;
            pass = s.inventory.resolveItemId(r.id, type, index) && s.inventory.countItem(type, index) >= r.value;
        } else if (r.type == "recipe") pass = known(s, r.id) == (r.state == "known");
        else if (r.type == "choice") pass = recorded(s, npc.id, r.id) == (r.state == "recorded");
        else if (r.type == "class") pass = s.hero.playerClass.id == r.id;
        else if (r.type == "race") {
            PlayerRaceDatabase races;
            const auto* entry = races.findById(r.id);
            pass = entry && entry->name == s.hero.race.name;
        } else if (r.type == "quest") {
            auto p = questProgress(s, r.id);
            if (r.state == "active") pass = p.active;
            else if (r.state == "objective") pass = p.objectiveComplete;
            else if (r.state == "completed") pass = p.completions > 0;
            else { auto staged = s; pass = acceptQuest(staged, r.id); }
        }
        if (!pass) {
            if (r.type == "flag") return "A different local decision or earlier conversation is required.";
            if (r.type == "recipe") return r.state == "unknown" ? "Recipe already known." : "Learn the recipe first.";
            if (r.type == "item") return "Requires " + std::to_string(r.value) + " of " + r.id + ".";
            if (r.type == "standing") return "Requires standing " + std::to_string(r.value) + " with " + r.id + ".";
            return "Requires " + r.type + ": " + r.id + (r.state.empty() ? "" : " (" + r.state + ")") + ".";
        }
    }
    return "";
}

bool chooseDialogue(GameSession& s, const DialogueResident& npc, const DialogueChoice& c, std::string& error) {
    error = dialogueLockReason(s, npc, c);
    if (!error.empty()) return false;
    // GameSession owns values, not controller references. Stage effects together,
    // then assign affected members in place to preserve live controller bindings.
    auto staged = s;
    for (const auto& r : c.effects) {
        if (r.type == "flag") {
            auto it = std::find_if(staged.story.flags.begin(), staged.story.flags.end(), [&](const StoryFlag& f) { return f.id == r.id; });
            if (it == staged.story.flags.end()) staged.story.flags.push_back({r.id, r.value});
            else it->value = r.value;
        } else if (r.type == "standing") {
            auto it = std::find_if(staged.story.factions.begin(), staged.story.factions.end(), [&](const FactionStanding& f) { return f.factionId == r.id; });
            if (it == staged.story.factions.end()) staged.story.factions.push_back({r.id, std::clamp(r.value, minStanding, maxStanding)});
            else it->standing = std::clamp(it->standing + r.value, minStanding, maxStanding);
        } else if (r.type == "recipe") {
            if (!craftingDatabase().find(r.id)) { error = "Unknown recipe."; return false; }
            if (!known(staged, r.id)) staged.story.knownRecipes.push_back(r.id);
        } else if (r.type == "quest") {
            if (!acceptQuest(staged, r.id)) { error = "This job cannot be accepted now; check its requirements and progress."; return false; }
        } else if (r.type == "item") {
            PlayerInventory::ItemType type; size_t index;
            if (!staged.inventory.resolveItemId(r.id, type, index) || !staged.inventory.tryAddItem(type, index, r.value)) {
                error = "The reward cannot fit in your backpack. Make room and try again."; return false;
            }
        } else { error = "Unsupported dialogue effect."; return false; }
    }
    if (c.once) staged.story.choices.push_back({npc.id, c.id, c.result});
    s.inventory = std::move(staged.inventory);
    s.story = std::move(staged.story);
    s.quests = std::move(staged.quests);
    s.guilds = std::move(staged.guilds);
    return true;
}

void openDialogue(GameSession& s, const DialogueResident& npc) {
    if (s.hero.currentLocation != npc.city || s.hero.currentLocationType != PeacefulCity) return;
    auto* node = npc.node(npc.entry);
    while (node) {
        std::vector<std::string> lines{npc.name + " - " + npc.role, node->text, ""};
        for (size_t i = 0; i < node->choices.size(); ++i) {
            const auto& c = node->choices[i];
            auto reason = dialogueLockReason(s, npc, c);
            lines.push_back(std::to_string(i + 1) + ". " + c.text + (reason.empty() ? "" : " [Unavailable: " + reason + "]"));
        }
        lines.push_back("0. Leave conversation");
        displayBorderedMenu(lines, "Choose: ");
        int selected = input(node->choices.size());
        if (!selected) return;
        const auto& c = node->choices[static_cast<size_t>(selected - 1)];
        std::string error;
        if (!chooseDialogue(s, npc, c, error)) { display::message(error); continue; }
        if (!c.result.empty()) display::message(c.result);
        node = c.next.empty() ? nullptr : npc.node(c.next);
    }
}

void openResidents(GameSession& s) {
    if (s.hero.currentLocationType != PeacefulCity) return;
    std::vector<const DialogueResident*> residents;
    for (const auto& r : dialogueDatabase().residents()) if (r.city == s.hero.currentLocation) residents.push_back(&r);
    while (true) {
        std::vector<std::string> lines{"Residents of " + s.hero.currentLocation};
        for (size_t i = 0; i < residents.size(); ++i) lines.push_back(std::to_string(i + 1) + ". " + residents[i]->name + " - " + residents[i]->role);
        lines.push_back("0. Back");
        displayBorderedMenu(lines, "Speak to: ");
        int selected = input(residents.size());
        if (!selected) return;
        openDialogue(s, *residents[static_cast<size_t>(selected - 1)]);
    }
}
