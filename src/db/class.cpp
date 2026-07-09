#include "class.hpp"

PlayerClassCollection::PlayerClassCollection() {
    templates = {
        {"Warrior", {75, 75, 0.05f, 0.02f, 8, 2, 10, 10, 0.05f, 1.5f, 0.05f, 1.0f, 1, 0.0f},
         {"Strong and resilient melee fighter, excels at absorbing damage."}},
        {"Knight", {70, 70, 0.08f, 0.03f, 10, 2, 8, 8, 0.04f, 1.4f, 0.05f, 1.0f, 1, 0.0f},
         {"Heavily armored, balanced between defense and offense."}},
        {"Berserker", {80, 80, 0.04f, 0.02f, 7, 2, 6, 6, 0.03f, 1.3f, 0.05f, 1.0f, 1, 0.0f},
         {"Frenzied melee attacker with high hitpoints and raw power."}},

        {"Assassin", {55, 55, 0.02f, 0.01f, 8, 2, 6, 6, 1.0f, 100.0f, 1.0f, 1.0f, 1, 0.0f},
         {"Agile and deadly, specializes in critical strikes."}},
        {"Rogue", {60, 60, 0.02f, 0.01f, 7, 2, 8, 8, 0.2f, 1.8f, 0.1f, 1.0f, 1, 0.0f},
         {"Stealthy fighter with high dexterity and precision."}},
        {"Ranger", {50, 50, 0.01f, 0.01f, 5, 4, 10, 10, 0.3f, 1.7f, 0.1f, 1.0f, 1, 0.0f},
         {"Skilled marksman, excels at ranged attacks and high crits."}},

        {"Mage", {45, 45, 0.01f, 0.05f, 2, 9, 18, 18, 0.05f, 1.5f, 0.05f, 1.2f, 1, 0.0f},
         {"Master of arcane arts, high magic attack and mana pool."}},
        {"Sorcerer", {40, 40, 0.01f, 0.07f, 2, 11, 22, 22, 0.05f, 1.5f, 0.05f, 1.3f, 1, 0.0f},
         {"Specialist in destructive spells, fragile but powerful."}},
        {"Scholar", {50, 50, 0.02f, 0.05f, 3, 7, 14, 14, 0.05f, 1.5f, 0.05f, 1.1f, 1, 0.0f},
         {"Versatile magic user, balanced between offense and defense."}},

        {"Shrewd Diplomat", {60, 60, 0.02f, 0.03f, 5, 4, 14, 14, 0.05f, 1.5f, 0.05f, 1.0f, 1, 0.0f},
         {"Uses wit and charm to outsmart opponents, prefers diplomacy over fighting."}},
        {"Travelling Merchant", {65, 65, 0.03f, 0.02f, 3, 2, 10, 10, 0.05f, 1.5f, 0.05f, 1.0f, 1, 0.0f},
         {"Journeys across lands trading goods, can defend himself when needed."}},
        {"Sheltered Noble", {55, 55, 0.02f, 0.05f, 3, 2, 10, 10, 0.05f, 1.5f, 0.05f, 1.0f, 1, 0.0f},
         {"Raised in comfort, trained in etiquette and light combat."}},
        {"Prisoner", {50, 50, 0.01f, 0.01f, 2, 2, 6, 6, 0.05f, 1.2f, 0.05f, 1.0f, 1, 0.0f},
         {"Imprisoned for unknown reasons, weak but resilient in spirit."}}
    };
}

const PlayerClassTemplate& PlayerClassCollection::getClassTemplate(const std::string& className) const {
    for (const auto& cls : templates) {
        if (cls.name == className) return cls;
    }
    throw std::runtime_error("Class template not found: " + className);
}
