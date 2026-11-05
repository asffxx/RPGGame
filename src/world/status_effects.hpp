#pragma once

#include <map>
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
// - Debuffs are applied through the meter model. Every combatant carries a
//   hidden 0..100 meter per debuff id. A landed hit carrying that debuff
//   fills the meter by `debuffMeterPerHit`, reduced by the target's armor
//   resistance; at 100 the debuff applies and the meter empties. Meters
//   decay each round.
namespace statuses {

// Points a landed hit adds to the target's meter for that debuff.
inline constexpr int debuffMeterPerHit = 20;
// Points every meter loses each combat round.
inline constexpr int debuffMeterDecayPerRound = 5;
// Hard cap on the bar, in points.
inline constexpr int debuffMeterMax = 100;

// Resistance derived from armor: half the armor fraction, capped at 50%.
// Armor 0.10 gives 0.05 resistance.
float armorResistance(float armor);

// Adds one hit of fill to the target's meter for `name`. Returns true and
// empties the meter when it fills; the caller then applies the debuff.
bool addDebuffMeter(std::map<std::string, int>& meters, const std::string& name, float resistance);

// Drops every meter by `points` (floored at 0) and erases empty entries.
void decayDebuffMeters(std::map<std::string, int>& meters, int points = debuffMeterDecayPerRound);

// "Burning [##--------] 20, Poison [----------] 0" trimmed to non-zero
// meters; empty string when nothing is tracked.
std::string describeDebuffMeters(const std::map<std::string, int>& meters);

const DebuffDatabase::DebuffDefinition* find(const std::string& nameOrId);

// Adds a stack or refreshes the duration. Returns false when the name is not
// in the catalog. The optional message describes the visible result.
bool apply(std::vector<ActiveDebuff>& active, const std::string& name,
           std::string* message = nullptr);

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
