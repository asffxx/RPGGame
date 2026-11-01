#pragma once

#include <string>
#include <vector>

// Class skills have their own catalog, separate from fixed store-bought spells.
enum class AbilityKind { PowerStrike, Brace, BloodRage, Execute, RiskyStrike, AimedShot,
                         ArcaneBolt, ManaBurst, Meditate, Distract, Supplies, Rally, LastStand };

struct ClassSkillEffects {
    float damageMultiplier = 1.0f;
    float alternateDamageMultiplier = 1.0f;
    float healthThreshold = 0.5f;
    float strongOutcomeChance = 0.5f;
    int healthCostPercent = 0;
    int manaCostPercent = 0;
    int manaRestorePercent = 0;
    float healFraction = 0.0f;
    float alternateHealFraction = 0.0f;
    float retaliationMultiplier = 1.0f;
    bool magical = false;
    bool undodgeable = false;
};

struct SignatureAbility {
    const char* className;
    const char* name;
    const char* description;
    AbilityKind kind;
    ClassSkillEffects effects{};
};

class ClassSkillDatabase {
public:
    ClassSkillDatabase();
    const std::vector<SignatureAbility>& getSkills() const { return skills; }
    const SignatureAbility& getClassSkill(const std::string& className) const;

private:
    std::vector<SignatureAbility> skills;
};

// Shared immutable catalog used by combat and character previews.
const SignatureAbility& signatureAbility(const std::string& className);
