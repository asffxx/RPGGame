#include "status_effects.hpp"

#include <algorithm>
#include <cmath>

namespace statuses {
namespace {

const DebuffDatabase& catalog() {
    static DebuffDatabase database;
    return database;
}

int stacksOf(const DebuffDatabase::DebuffDefinition& definition,
             const ActiveDebuff& active) {
    return std::max(1, std::min(active.stacks, definition.stacking.maxStacks));
}

bool atMaximum(const DebuffDatabase::DebuffDefinition& definition,
               const ActiveDebuff& active) {
    return definition.stacking.stackable &&
           stacksOf(definition, active) >= definition.stacking.maxStacks;
}

} // namespace

const DebuffDatabase::DebuffDefinition* find(const std::string& nameOrId) {
    return catalog().find(nameOrId);
}

float applicationChance(const std::string& name) {
    const auto* definition = find(name);
    return definition ? definition->application.effectChance : 1.0f;
}

bool apply(std::vector<ActiveDebuff>& active, const std::string& name,
           std::string* message) {
    const auto* definition = find(name);
    if (!definition) return false;
    if (message) message->clear();

    auto existing = std::find_if(active.begin(), active.end(),
                                 [&](const ActiveDebuff& entry) { return entry.name == definition->name; });
    if (existing == active.end()) {
        const int duration = std::max(1, definition->effect.durationTurns);
        active.push_back({definition->name, duration, 1});
        if (message) *message = "applied " + definition->name;
        return true;
    }

    int nextStacks = existing->stacks;
    if (definition->stacking.stackable)
        nextStacks = std::min(definition->stacking.maxStacks, existing->stacks + 1);
    else
        nextStacks = 1;

    int refresh = std::max(1, definition->effect.durationTurns);
    if (nextStacks > 1) refresh += std::max(0, definition->effect.addedTurnOnTrigger);
    existing->stacks = nextStacks;
    existing->remainingTurns = std::max(existing->remainingTurns, refresh);
    if (message) {
        *message = definition->name + " x" + std::to_string(nextStacks);
        if (atMaximum(*definition, *existing) && !definition->stacking.maxStackEffectName.empty())
            *message += " (" + definition->stacking.maxStackEffectName + ")";
    }
    return true;
}

bool applyRoll(std::vector<ActiveDebuff>& active, const std::string& name, float roll,
               std::string* message) {
    if (roll < 0.0f || roll >= 1.0f) roll = 0.0f;
    if (roll >= applicationChance(name)) return false;
    return apply(active, name, message);
}

std::vector<std::string> tick(std::vector<ActiveDebuff>& active, int& hitpoints, int& mana) {
    std::vector<std::string> lines;
    for (auto it = active.begin(); it != active.end();) {
        const auto* definition = find(it->name);
        if (!definition || it->remainingTurns <= 0) {
            it = active.erase(it);
            continue;
        }
        const int stacks = stacksOf(*definition, *it);
        const float amplifier = atMaximum(*definition, *it)
            ? std::max(0.0f, definition->effect.amplifierOnTrigger) : 1.0f;
        const int damage = static_cast<int>(std::lround(
            std::max(0, definition->effect.healthDecrease) * stacks * amplifier));
        const int drain = static_cast<int>(std::lround(
            std::max(0, definition->effect.manaDecrease) * stacks * amplifier));
        if (damage > 0) {
            hitpoints = std::max(0, hitpoints - damage);
            lines.push_back("takes " + std::to_string(damage) + " damage from " +
                            definition->name + " x" + std::to_string(stacks) + ".");
        }
        if (drain > 0) {
            mana = std::max(0, mana - drain);
            lines.push_back("loses " + std::to_string(drain) + " mana to " +
                            definition->name + " x" + std::to_string(stacks) + ".");
        }

        it->remainingTurns--;
        if (it->remainingTurns <= 0) {
            lines.push_back(definition->name + " fades.");
            it = active.erase(it);
        } else {
            ++it;
        }
    }
    return lines;
}

int attackPenalty(const std::vector<ActiveDebuff>& active) {
    int total = 0;
    for (const auto& entry : active) {
        const auto* definition = find(entry.name);
        if (!definition) continue;
        total += std::max(0, definition->effect.attackDecrease) * stacksOf(*definition, entry);
    }
    return total;
}

int magicAttackPenalty(const std::vector<ActiveDebuff>& active) {
    int total = 0;
    for (const auto& entry : active) {
        const auto* definition = find(entry.name);
        if (!definition) continue;
        total += std::max(0, definition->effect.magicAttackDecrease) * stacksOf(*definition, entry);
    }
    return total;
}

float armorMultiplier(const std::vector<ActiveDebuff>& active) {
    float factor = 1.0f;
    for (const auto& entry : active) {
        const auto* definition = find(entry.name);
        if (!definition) continue;
        const int reduction = std::max(0, definition->effect.defenseDecrease) * stacksOf(*definition, entry);
        factor *= std::max(0.0f, 1.0f - static_cast<float>(reduction) / 100.0f);
    }
    return factor;
}

float magicArmorMultiplier(const std::vector<ActiveDebuff>& active) {
    float factor = 1.0f;
    for (const auto& entry : active) {
        const auto* definition = find(entry.name);
        if (!definition) continue;
        const int reduction = std::max(0, definition->effect.magicDefenseDecrease) * stacksOf(*definition, entry);
        factor *= std::max(0.0f, 1.0f - static_cast<float>(reduction) / 100.0f);
    }
    return factor;
}

float incomingDamageMultiplier(const std::vector<ActiveDebuff>& active) {
    float factor = 1.0f;
    for (const auto& entry : active) {
        const auto* definition = find(entry.name);
        if (!definition) continue;
        if (atMaximum(*definition, entry))
            factor *= std::max(0.0f, definition->effect.amplifierOnTrigger);
    }
    return factor;
}

std::string describe(const std::vector<ActiveDebuff>& active) {
    std::string result;
    for (const auto& entry : active) {
        const auto* definition = find(entry.name);
        if (!result.empty()) result += ", ";
        result += entry.name;
        if (entry.stacks > 1) result += " x" + std::to_string(entry.stacks);
        if (definition && atMaximum(*definition, entry) &&
            !definition->stacking.maxStackEffectName.empty())
            result += " [" + definition->stacking.maxStackEffectName + "]";
        result += " (" + std::to_string(entry.remainingTurns) + " turns)";
    }
    return result;
}

void clear(std::vector<ActiveDebuff>& active) {
    active.clear();
}

} // namespace statuses
