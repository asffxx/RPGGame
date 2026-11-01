#include "class_skill.hpp"

#include <stdexcept>

ClassSkillDatabase::ClassSkillDatabase() {
    auto add = [&](const char* className, const char* name, const char* description, AbilityKind kind) -> ClassSkillEffects& {
        skills.push_back({className, name, description, kind, {}});
        return skills.back().effects;
    };

    add("Warrior", "Power Strike", "Strike for 175% physical damage.", AbilityKind::PowerStrike).damageMultiplier = 1.75f;
    add("Knight", "Brace", "Strike normally; halve the next enemy retaliation.", AbilityKind::Brace).retaliationMultiplier = 0.5f;
    {
        auto& e = add("Berserker", "Blood Rage", "Spend 10% max HP (cannot self-kill); strike for 250% physical damage.", AbilityKind::BloodRage);
        e.healthCostPercent = 10;
        e.damageMultiplier = 2.5f;
    }
    {
        auto& e = add("Assassin", "Execute", "Strike for 250% physical damage below half enemy HP, otherwise 125%.", AbilityKind::Execute);
        e.damageMultiplier = 2.5f;
        e.alternateDamageMultiplier = 1.25f;
        e.healthThreshold = 0.5f;
    }
    {
        auto& e = add("Rogue", "Risky Strike", "50% chance of 300% physical damage, otherwise 50%.", AbilityKind::RiskyStrike);
        e.damageMultiplier = 3.0f;
        e.alternateDamageMultiplier = 0.5f;
        e.strongOutcomeChance = 0.5f;
    }
    {
        auto& e = add("Ranger", "Aimed Shot", "Strike for 150% physical damage; cannot be dodged.", AbilityKind::AimedShot);
        e.damageMultiplier = 1.5f;
        e.undodgeable = true;
    }
    {
        auto& e = add("Mage", "Arcane Bolt", "Deal 175% magic damage without spending mana.", AbilityKind::ArcaneBolt);
        e.damageMultiplier = 1.75f;
        e.magical = true;
    }
    {
        auto& e = add("Sorcerer", "Mana Burst", "Spend half current mana; deal 200% magic damage plus mana spent.", AbilityKind::ManaBurst);
        e.damageMultiplier = 2.0f;
        e.magical = true;
        e.manaCostPercent = 50;
    }
    add("Scholar", "Meditate", "Restore 40% maximum mana (at least 1).", AbilityKind::Meditate).manaRestorePercent = 40;
    add("Shrewd Diplomat", "Distract", "Prevent the next enemy retaliation.", AbilityKind::Distract);
    add("Travelling Merchant", "Emergency Supplies", "Restore 30% maximum HP (at least 1).", AbilityKind::Supplies).healFraction = 0.3f;
    add("Sheltered Noble", "Rally", "Restore 20% maximum HP to yourself and each living companion.", AbilityKind::Rally).healFraction = 0.2f;
    {
        auto& e = add("Prisoner", "Last Stand", "Restore 40% maximum HP below half HP, otherwise 20%.", AbilityKind::LastStand);
        e.healFraction = 0.4f;
        e.alternateHealFraction = 0.2f;
        e.healthThreshold = 0.5f;
    }
}

const SignatureAbility& ClassSkillDatabase::getClassSkill(const std::string& className) const {
    for (const auto& skill : skills) if (className == skill.className) return skill;
    throw std::invalid_argument("No signature ability for class: " + className);
}

const SignatureAbility& signatureAbility(const std::string& className) {
    static const ClassSkillDatabase database;
    return database.getClassSkill(className);
}
