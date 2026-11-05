#include "status_effects.hpp"

#include <algorithm>
#include <cmath>
#include <map>

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

float armorResistance(float armor) {
    if (!std::isfinite(armor) || armor <= 0.0f) return 0.0f;
    return std::min(0.5f, armor * 0.5f);
}

bool addDebuffMeter(std::map<std::string, int>& meters, const std::string& name, float resistance) {
    const auto* definition = find(name);
    if (!definition) return false;
    const float clamped = std::isfinite(resistance) ? std::clamp(resistance, 0.0f, 1.0f) : 0.0f;
    const int fill = std::max(1, static_cast<int>(std::lround(
        debuffMeterPerHit * (1.0f - clamped))));
    int& value = meters[definition->name];
    value += fill;
    if (value < debuffMeterMax) return false;
    meters.erase(definition->name);
    return true;
}

void decayDebuffMeters(std::map<std::string, int>& meters, int points) {
    if (points <= 0) return;
    for (auto it = meters.begin(); it != meters.end();) {
        it->second -= points;
        if (it->second <= 0) it = meters.erase(it);
        else ++it;
    }
}

std::string describeDebuffMeters(const std::map<std::string, int>& meters) {
    std::string result;
    for (const auto& [name, value] : meters) {
        if (value <= 0) continue;
        const int clamped = std::min(value, debuffMeterMax);
        const int filled = clamped * 10 / debuffMeterMax;
        if (!result.empty()) result += ", ";
        result += name + " [" + std::string(filled, '#') +
                  std::string(10 - filled, '-') + "] " + std::to_string(clamped);
    }
    return result;
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
