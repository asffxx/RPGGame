#include "abilities.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {
int percentage(int amount, int percent) {
    return static_cast<int>(static_cast<std::int64_t>(amount) * percent / 100);
}

int heal(Stats& stats, float portion) {
    if (stats.hitpoints <= 0) return 0;
    int amount = std::min(std::max(1, static_cast<int>(stats.maxHitpoints * portion)),
                          std::max(0, stats.maxHitpoints - stats.hitpoints));
    stats.hitpoints += amount;
    return amount;
}
}

float AbilityState::consumeRetaliation() {
    float multiplier = distracted ? 0.0f : (braced ? retaliationMultiplier : 1.0f);
    distracted = false;
    braced = false;
    retaliationMultiplier = 1.0f;
    return multiplier;
}

bool useSignature(Player& player, std::vector<NPC>& party, Enemy& enemy,
                  AbilityState& state, float hitRoll, float effectRoll, std::string& log) {
    if (state.used || player.stats.hitpoints <= 0 || enemy.stats.data.hitpoints <= 0) return false;
    const auto& ability = signatureAbility(player.playerClass.name);
    const auto& effects = ability.effects;
    auto& stats = player.stats;
    state.used = true;
    log = player.name + " used " + ability.name;
    float multiplier = effects.damageMultiplier;
    int extra = 0;
    switch (ability.kind) {
        case AbilityKind::PowerStrike:
        case AbilityKind::AimedShot:
        case AbilityKind::ArcaneBolt: break;
        case AbilityKind::Brace:
            state.braced = true;
            state.retaliationMultiplier = effects.retaliationMultiplier;
            break;
        case AbilityKind::BloodRage: {
            int cost = std::min(stats.hitpoints - 1, std::max(1, percentage(stats.maxHitpoints, effects.healthCostPercent)));
            stats.hitpoints -= cost;
            log += " (spent " + std::to_string(cost) + " HP)";
            break;
        }
        case AbilityKind::Execute:
            multiplier = enemy.stats.data.hitpoints < enemy.stats.data.maxHitpoints * effects.healthThreshold
                ? effects.damageMultiplier : effects.alternateDamageMultiplier;
            break;
        case AbilityKind::RiskyStrike:
            multiplier = effectRoll < effects.strongOutcomeChance ? effects.damageMultiplier : effects.alternateDamageMultiplier;
            break;
        case AbilityKind::ManaBurst:
            extra = percentage(stats.mana, effects.manaCostPercent); stats.mana -= extra;
            log += " (spent " + std::to_string(extra) + " mana)";
            break;
        case AbilityKind::Meditate: {
            int restore = std::min(std::max(1, percentage(stats.maxMana, effects.manaRestorePercent)), std::max(0, stats.maxMana - stats.mana));
            stats.mana += restore;
            log += " and restored " + std::to_string(restore) + " mana.";
            return true;
        }
        case AbilityKind::Distract:
            state.distracted = true; log += "; the enemy will miss its next turn."; return true;
        case AbilityKind::Supplies:
            log += " and recovered " + std::to_string(heal(stats, effects.healFraction)) + " HP."; return true;
        case AbilityKind::Rally: {
            int restored = heal(stats, effects.healFraction);
            for (auto& npc : party) restored += heal(npc.stats, effects.healFraction);
            log += " and restored " + std::to_string(restored) + " HP across the party.";
            return true;
        }
        case AbilityKind::LastStand: {
            float portion = stats.hitpoints < stats.maxHitpoints * effects.healthThreshold
                ? effects.healFraction : effects.alternateHealFraction;
            log += " and recovered " + std::to_string(heal(stats, portion)) + " HP."; return true;
        }
    }
    if (!effects.undodgeable && hitRoll < enemy.stats.data.dodgeRate) {
        log += ", but the enemy dodged.";
        return true;
    }
    float defense = effects.magical ? enemy.stats.data.magicArmor : enemy.stats.data.armor;
    int attack = effects.magical ? stats.magicAttack : stats.attack;
    int damage = static_cast<int>(std::round((attack * multiplier + extra) * (1.0f - std::clamp(defense, 0.0f, 1.0f))));
    enemy.stats.data.hitpoints = std::max(0, enemy.stats.data.hitpoints - damage);
    log += " and dealt " + std::to_string(damage) + " damage.";
    return true;
}
