#include "class_skill.hpp"
#include "content.hpp"

#include <stdexcept>

ClassSkillDatabase::ClassSkillDatabase()
    : skills(content::catalog<SignatureAbility>("characters/skills.json")) {}

const SignatureAbility& ClassSkillDatabase::getClassSkill(const std::string& className) const {
    for (const auto& skill : skills) if (className == skill.className) return skill;
    throw std::invalid_argument("No signature ability for class: " + className);
}

const SignatureAbility& signatureAbility(const std::string& className) {
    static const ClassSkillDatabase database;
    return database.getClassSkill(className);
}
