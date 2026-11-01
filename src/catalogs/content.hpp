#pragma once

// JSON-to-existing-struct adapters. Gameplay continues to use its original
// catalogs and interfaces; this layer only replaces their initialization.
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <type_traits>
#include "core/ids.hpp"
#include "core/paths.hpp"
#include "gameplay/datafile.hpp"
#include "catalogs/equipment.hpp"
#include "catalogs/potion.hpp"
#include "catalogs/food.hpp"
#include "catalogs/spell.hpp"
#include "catalogs/location.hpp"
#include "catalogs/class.hpp"
#include "catalogs/race.hpp"
#include "catalogs/debuff.hpp"
#include "catalogs/event.hpp"
#include "catalogs/class_skill.hpp"
#include "catalogs/guild.hpp"
#include "catalogs/rumor.hpp"
#include "entities/enemy.hpp"

namespace content {

// The same complete field map is used by the migration snapshot verifier.
// No content values or fallback records live in this file.
#define FIELD(member) a(#member, v.member)
template<class A> void fields(A& a, EquipmentandWeaponDatabase::equipmentProperties& v) {
    FIELD(name); FIELD(type); FIELD(defenseIncrease); FIELD(magicDefenseIncrease);
    FIELD(attackIncrease); FIELD(magicAttackIncrease); FIELD(critRateIncrease); FIELD(critDamageIncrease);
    FIELD(debuffs); FIELD(debuffChance); FIELD(effectDesc);
    FIELD(pricePlatinum); FIELD(priceGold); FIELD(priceSilver); FIELD(priceCopper); FIELD(id);
}
template<class A> void fields(A& a, PotionDatabase::potionProperties& v) {
    FIELD(name); FIELD(hpEffect); FIELD(attackEffect); FIELD(defenseEffect); FIELD(magicEffect);
    FIELD(manaRestore); FIELD(manaIncreaseMax); FIELD(magicDefenseEffect); FIELD(critRateEffect);
    FIELD(critDamageEffect); FIELD(durationTurns); FIELD(effectDesc);
    FIELD(pricePlatinum); FIELD(priceGold); FIELD(priceSilver); FIELD(priceCopper); FIELD(id);
}
template<class A> void fields(A& a, FoodandDrinksDatabase::FoodsandDrinksProperties& v) {
    FIELD(name); FIELD(type); FIELD(healthRestoration); FIELD(maxHealthBuffBonus);
    FIELD(maxManaBuffBonus); FIELD(manaRestoration); FIELD(itemDesc);
    FIELD(pricePlatinum); FIELD(priceGold); FIELD(priceSilver); FIELD(priceCopper); FIELD(id);
}
template<class A> void fields(A& a, SpellDatabase::SpellData& v) {
    FIELD(spellName); FIELD(manaCost); FIELD(healthDamage); FIELD(manaDamage);
    FIELD(healthRestore); FIELD(manaRestore); FIELD(armorIncrease); FIELD(magicArmorIncrease);
    FIELD(armorReduction); FIELD(magicArmorReduction); FIELD(hasDebuff); FIELD(debuffs);
    FIELD(debuffModifierOnHit); FIELD(applicableOnStaffs); FIELD(staffBonusModifier);
    FIELD(requiredLevel); FIELD(description);
    FIELD(pricePlatinum); FIELD(priceGold); FIELD(priceSilver); FIELD(priceCopper); FIELD(id);
}
template<class A> void fields(A& a, Stats& v) {
    FIELD(hitpoints); FIELD(maxHitpoints); FIELD(armor); FIELD(magicArmor); FIELD(attack);
    FIELD(magicAttack); FIELD(mana); FIELD(maxMana); FIELD(critRate); FIELD(critDamage);
    FIELD(dodgeRate); FIELD(magicAmplifierRate); FIELD(level); FIELD(expe);
}
template<class A> void fields(A& a, Lore& v) { FIELD(description); }
template<class A> void fields(A& a, RaceLore& v) { FIELD(description); }
template<class A> void fields(A& a, PlayerClassTemplate& v) { FIELD(name); FIELD(stats); FIELD(lore); FIELD(id); }
template<class A> void fields(A& a, PlayerRaceTemplate& v) { FIELD(name); FIELD(statBonus); FIELD(lore); FIELD(id); }
template<class A> void fields(A& a, locationDatabase::locationProperties& v) {
    FIELD(name); FIELD(description); FIELD(difficultyLevel); FIELD(type); FIELD(id);
}
template<class A> void fields(A& a, Economy& v) { FIELD(platinum); FIELD(gold); FIELD(silver); FIELD(copper); }
template<class A> void fields(A& a, EnemyStats::StatsData& v) {
    FIELD(hitpoints); FIELD(maxHitpoints); FIELD(armor); FIELD(magicArmor); FIELD(attack);
    FIELD(magicAttack); FIELD(mana); FIELD(maxMana); FIELD(critRate); FIELD(critDamage);
    FIELD(dodgeRate); FIELD(magicAmplifierRate); FIELD(level); FIELD(expe); FIELD(economy);
}
template<class A> void fields(A& a, EnemyStats& v) { FIELD(data); FIELD(scaled); }
template<class A> void fields(A& a, enemyLore& v) { FIELD(desc); }
template<class A> void fields(A& a, EnemyDatabase::EnemyTemplate& v) {
    FIELD(name); FIELD(stats); FIELD(lore); FIELD(minLevel); FIELD(debuffs);
    FIELD(difficultyLevel); FIELD(allowedLocations); FIELD(id);
}
template<class A> void fields(A& a, debuffSettings::DebuffEffect& v) {
    FIELD(healthDecrease); FIELD(attackDecrease); FIELD(defenseDecrease); FIELD(magicAttackDecrease);
    FIELD(magicDefenseDecrease); FIELD(durationTurns); FIELD(damageProc); FIELD(addedTurnOnTrigger); FIELD(amplifierOnTrigger);
}
template<class A> void fields(A& a, debuffSettings::DebuffStackRules& v) {
    FIELD(stackable); FIELD(maxStacks); FIELD(maxStackEffectName);
}
template<class A> void fields(A& a, debuffSettings::DebuffApplication& v) {
    FIELD(throwable); FIELD(enchantOnWeapon); FIELD(nameOnEnchantment); FIELD(effectChance);
}
template<class A> void fields(A& a, DebuffDatabase::DebuffDefinition& v) {
    FIELD(name); FIELD(effect); FIELD(stacking); FIELD(application); FIELD(effectDesc);
}
template<class A> void fields(A& a, eventDatabase::eventProperties& v) {
    FIELD(name); FIELD(description); FIELD(goldReward); FIELD(expReward); FIELD(healthEffect);
    FIELD(manaEffect); FIELD(enemyEncounter); FIELD(enemyName); FIELD(enemyCount);
}
template<class A> void fields(A& a, ClassSkillEffects& v) {
    FIELD(damageMultiplier); FIELD(alternateDamageMultiplier); FIELD(healthThreshold); FIELD(strongOutcomeChance);
    FIELD(healthCostPercent); FIELD(manaCostPercent); FIELD(manaRestorePercent); FIELD(healFraction);
    FIELD(alternateHealFraction); FIELD(retaliationMultiplier); FIELD(magical); FIELD(undodgeable);
}
template<class A> void fields(A& a, SignatureAbility& v) {
    FIELD(className); FIELD(name); FIELD(description); FIELD(kind); FIELD(effects);
}
template<class A> void fields(A& a, QuestDefinition& v) {
    FIELD(id); FIELD(title); FIELD(briefing); FIELD(location); FIELD(enemyName); FIELD(completionText);
    FIELD(rewardSilver); FIELD(rewardExperience); FIELD(category); FIELD(minimumLevel); FIELD(offeringGuild); FIELD(prerequisite);
}
template<class A> void fields(A& a, GuildDefinition& v) {
    FIELD(id); FIELD(name); FIELD(city); FIELD(introduction); FIELD(quests);
}
template<class A> void fields(A& a, RumorDefinition& v) {
    FIELD(id); FIELD(city); FIELD(destination); FIELD(text);
}
#undef FIELD

inline int enumMaximum(LocationType) { return static_cast<int>(PeacefulCity); }
inline int enumMaximum(AbilityKind) { return static_cast<int>(AbilityKind::LastStand); }
inline int enumMaximum(QuestCategory) { return static_cast<int>(QuestCategory::Commission); }

// SignatureAbility retains its const-char interface. Interned strings are
// immutable and live for the process lifetime, including across catalog copies.
inline const char* intern(const std::string& value) {
    static std::set<std::string> strings;
    return strings.insert(value).first->c_str();
}

class Reader {
public:
    Reader(const datafile::Value& object, std::string where) : object(object), where(std::move(where)) {}
    template<class T> void operator()(const char* name, T& result) {
        used.insert(name);
        const std::string field = where + "." + name;
        try { decode(object.at(name), result, field); }
        catch (const std::exception& e) { throw std::runtime_error(field + ": " + e.what()); }
    }
    void finish() const {
        for (const auto& entry : object.object())
            if (!used.count(entry.first)) throw std::runtime_error(where + "." + entry.first + ": unknown field");
    }
    template<class T> static void decode(const datafile::Value& value, T& out, const std::string& where) {
        if constexpr (std::is_same_v<T, std::string>) out = value.string();
        else if constexpr (std::is_same_v<T, const char*>) out = intern(value.string());
        else if constexpr (std::is_same_v<T, bool>) out = value.boolean();
        else if constexpr (std::is_enum_v<T>) {
            int n = value.integer();
            if (n < 0 || n > enumMaximum(T{})) throw std::runtime_error("invalid enum value");
            out = static_cast<T>(n);
        } else if constexpr (std::is_integral_v<T>) out = value.integer();
        else if constexpr (std::is_floating_point_v<T>) {
            double n = value.number();
            if (!std::isfinite(n) || std::abs(n) > std::numeric_limits<T>::max()) throw std::runtime_error("number out of range");
            out = static_cast<T>(n);
        } else { Reader reader(value, where); fields(reader, out); reader.finish(); }
    }
    template<class T> static void decode(const datafile::Value& value, std::vector<T>& out, const std::string& where) {
        const auto& array = value.array();
        if (array.size() > 10000) throw std::runtime_error("too many records");
        out.clear();
        for (size_t i = 0; i < array.size(); ++i) {
            T entry{};
            decode(array[i], entry, where + "[" + std::to_string(i) + "]");
            out.push_back(std::move(entry));
        }
    }
private:
    const datafile::Value& object;
    std::string where;
    std::set<std::string> used;
};

template<class T, class = void> struct HasId : std::false_type {};
template<class T> struct HasId<T, std::void_t<decltype(std::declval<T>().id)>> : std::true_type {};

template<class T> std::vector<T> readCatalog(const std::string& filename, bool allowEmpty = false) {
    const auto path = paths::dataFile(filename);
    datafile::Value root;
    std::string error;
    if (!datafile::loadFile(path, root, error)) throw std::runtime_error(error);
    try {
        if (root.at("schemaVersion").integer() != 1) throw std::runtime_error("unsupported schemaVersion");
        std::vector<T> result;
        Reader::decode(root.at("records"), result, "records");
        if (!allowEmpty && result.empty()) throw std::runtime_error("records: empty catalog");
        if constexpr (HasId<T>::value) {
            std::set<std::string> ids;
            for (const auto& record : result)
                if (!ids::valid(record.id) || !ids.insert(record.id).second)
                    throw std::runtime_error("records.id: invalid or duplicate id " + record.id);
        }
        return result;
    } catch (const std::exception& e) { throw std::runtime_error(path.string() + ": " + e.what()); }
}

template<class T> const std::vector<T>& catalog(const std::string& filename, bool allowEmpty = false) {
    static std::map<std::string, std::vector<T>> cache;
    auto found = cache.find(filename);
    if (found == cache.end()) found = cache.emplace(filename, readCatalog<T>(filename, allowEmpty)).first;
    return found->second;
}

// Validates references after leaf catalogs load, avoiding constructor cycles.
void validateCatalogs();
} // namespace content
