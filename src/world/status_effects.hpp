#pragma once

#include <string>
#include <vector>

#include "catalogs/debuff.hpp"
#include "core/types.hpp"

// One duration and stacking model shared by every debuff source.
//
// Model:
// - A debuff has a base duration in combat rounds and up to maxStacks stacks.
// - Applying an existing debuff adds a stack (when stackable and below the
//   cap) and refreshes the duration. addedTurnOnTrigger extends the refresh.
// - Each round, healthDecrease and manaDecrease apply per stack. At maximum
//   stacks, amplifierOnTrigger multiplies that round's damage or drain.
// - attack/magicAttack decrease is flat per stack. defense decreases are
//   percentage points of the target's armor per stack.
// - Durations tick down after their effect fires. Zero stacks remove the
//   debuff.
namespace statuses {

const DebuffDatabase::DebuffDefinition* find(const std::string& nameOrId);

// Adds a stack or refreshes the duration. Returns false when the name is not
// in the catalog. The optional message describes the visible result.
bool apply(std::vector<ActiveDebuff>& active, const std::string& name,
           std::string* message = nullptr);

// Deterministic application used by combat and tests. `roll` is in [0, 1).
// Returns true when the debuff was applied.
bool applyRoll(std::vector<ActiveDebuff>& active, const std::string& name, float roll,
               std::string* message = nullptr);

// Catalog application chance, or 1.0 when the debuff is unknown.
float applicationChance(const std::string& name);

// Applies one round of damage, mana drain, and duration ticking. Removes
// expired entries and returns player-facing log lines.
std::vector<std::string> tick(std::vector<ActiveDebuff>& active, int& hitpoints, int& mana);

int attackPenalty(const std::vector<ActiveDebuff>& active);
int magicAttackPenalty(const std::vector<ActiveDebuff>& active);

// Multiplicative armor factor, e.g. 0.9 for a 10% defense reduction.
float armorMultiplier(const std::vector<ActiveDebuff>& active);
float magicArmorMultiplier(const std::vector<ActiveDebuff>& active);

// Damage-taken multiplier from debuffs at maximum stacks.
float incomingDamageMultiplier(const std::vector<ActiveDebuff>& active);

// "Poison x2 (2 turns)" or an empty string.
std::string describe(const std::vector<ActiveDebuff>& active);

void clear(std::vector<ActiveDebuff>& active);

} // namespace statuses
