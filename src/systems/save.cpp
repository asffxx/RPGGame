#include "save.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <stdexcept>
#include <type_traits>
#include "data/class.hpp"
#include "data/race.hpp"
#include "data/location.hpp"
#include "systems/guild.hpp"
#include "world/quest_system.hpp"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace {
constexpr size_t maxEntries = 10000;
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

// One field list serves both reading and writing, avoiding asymmetric formats.
template<class A> void fields(A& a, Stats& s) {
    a(s.hitpoints, s.maxHitpoints, s.armor, s.magicArmor, s.attack, s.magicAttack,
      s.mana, s.maxMana, s.critRate, s.critDamage, s.dodgeRate,
      s.magicAmplifierRate, s.level, s.expe);
}
template<class A> void fields(A& a, Lore& v) { a(v.description); }
template<class A> void fields(A& a, PlayerRace& v) { a(v.name, v.lore); }
template<class A> void fields(A& a, PlayerClassTemplate& v) { a(v.name, v.stats, v.lore); }
template<class A> void fields(A& a, Economy& v) { a(v.platinum, v.gold, v.silver, v.copper); }
template<class A> void fields(A& a, Progression& v) { a(v.level, v.exp, v.skillPoints, v.experienceRequired); }
template<class A> void fields(A& a, TimeSystem& v) {
    int turn = v.getCurrentTurn(), period = static_cast<int>(v.getCurrentPeriod()), days = v.getTotalDays();
    a(turn, period, days);
    require(turn >= 0 && turn <= 100000000 && period >= 0 && period < 4 &&
            days >= 0 && days == turn / 4 && period == turn % 4, "Invalid time state.");
    v.setState(turn, static_cast<TimeSystem::TimePeriod>(period), days);
}
template<class A> void fields(A& a, Player& v) {
    a(v.name, v.stats, v.economy, v.progression, v.race, v.playerClass, v.timeSystem,
      v.debuffs, v.equippedWeaponDebuffs, v.equippedWeaponDebuffChance,
      v.currentLocationType, v.currentLocation, v.learnedSpells, v.equippedStaffSpells,
      v.sleptToday, v.defeatedEnemies, v.discoveredLocations, v.boughtWeapons,
      v.hiredSpecialCharacters, v.encounteredEvents, v.hasNewDictionaryEntry);
}
template<class A> void fields(A& a, NPC& v) {
    a(v.name, v.stats, v.race, v.playerClass, v.level, v.inventory, v.equippedWeapon,
      v.equippedArmor, v.debuffs, v.spells, v.wagePerWeek, v.story, v.dialogues);
}
template<class A> void fields(A& a, PlayerInventory::InventoryItem& v) {
    a(v.type, v.dbIndex, v.quantity, v.enchanted);
}
template<class A> void fields(A& a, PlayerInventory::Buff& v) {
    a(v.name, v.attackBonus, v.defenseBonus, v.magicAttackBonus, v.magicDefenseBonus,
      v.maxManaBonus, v.critRateBonus, v.critDamageBonus, v.remainingTurns);
}
template<class A> void fields(A& a, PlayerInventory& v) {
    a(v.inventory, v.equipped.armorIndex, v.equipped.weaponIndex, v.equipped.staffIndex);
    bool active = v.activeBuff.has_value();
    a(active);
    if (active) {
        if (!v.activeBuff) v.activeBuff.emplace("", 0, 0, 0, 0, 0, 0, 0, 0);
        a(*v.activeBuff);
    } else v.activeBuff.reset();
}
template<class A> void fields(A& a, GuildProgress& v) {
    a(v.guildId, v.joined, v.completedQuests, v.active, v.objectiveComplete);
}
template<class A> void fields(A& a, RumorHistory& v) { a(v.tavern, v.rumorId); }
template<class A> void fields(A& a, GuildState& v) { a(v.guilds, v.cityLeads, v.rumors); }
template<class A> void fields(A& a, QuestProgress& v) {
    a(v.questId, v.active, v.objectiveComplete, v.completions, v.acceptingGuild);
}
template<class A> void fields(A& a, GameSession& v) {
    a(v.hero, v.inventory, v.party, v.travel.discovered, v.travel.marked,
      v.actionCounter, v.lastWeekPaid);
    if (a.version >= 2) {
        std::string section = "GUILD_STATE";
        a(section);
        require(section == "GUILD_STATE", "Missing guild save section.");
        a(v.guilds);
    }
    if (a.version >= 3) {
        std::string section = "QUEST_STATE";
        a(section);
        require(section == "QUEST_STATE", "Missing quest save section.");
        a(v.quests.jobs);
    }
}

struct Writer {
    std::ostream& stream;
    int version = 3;
    template<class... T> void operator()(T&... v) { (value(v), ...); }
    template<class T> void value(T& v) {
        if constexpr (std::is_enum_v<T>) stream << static_cast<int>(v) << '\n';
        else if constexpr (std::is_arithmetic_v<T>) stream << v << '\n';
        else fields(*this, v);
    }
    void value(std::string& v) {
        require(v.size() <= 65536, "Saved string is too long.");
        stream << std::quoted(v) << '\n';
    }
    template<class T> void value(std::vector<T>& v) {
        require(v.size() <= maxEntries, "Too many saved entries.");
        stream << v.size() << '\n';
        for (auto& entry : v) value(entry);
    }
    template<class T> void value(std::set<T>& v) {
        require(v.size() <= maxEntries, "Too many saved entries.");
        stream << v.size() << '\n';
        for (auto entry : v) value(entry);
    }
};
struct Reader {
    std::istream& stream;
    int version = 3;
    template<class... T> void operator()(T&... v) { (value(v), ...); }
    template<class T> void value(T& v) {
        if constexpr (std::is_enum_v<T>) {
            int n = 0; value(n); v = static_cast<T>(n);
        } else if constexpr (std::is_arithmetic_v<T>) {
            require(static_cast<bool>(stream >> v), "Invalid or truncated save.");
            if constexpr (std::is_floating_point_v<T>) require(std::isfinite(v), "Invalid numeric value.");
        } else fields(*this, v);
    }
    void value(std::string& v) {
        stream >> std::ws;
        require(stream.peek() == '"', "Invalid string in save.");
        require(static_cast<bool>(stream >> std::quoted(v)) && v.size() <= 65536, "Invalid saved string.");
    }
    template<class T> void value(std::vector<T>& v) {
        size_t count = 0; value(count);
        require(count <= maxEntries, "Too many saved entries.");
        v.resize(count);
        for (auto& entry : v) value(entry);
    }
    template<class T> void value(std::set<T>& v) {
        size_t count = 0; value(count);
        require(count <= maxEntries, "Too many saved entries.");
        v.clear();
        for (size_t i = 0; i < count; ++i) {
            T entry{}; value(entry);
            require(v.insert(entry).second, "Duplicate saved entry.");
        }
    }
};

void validateStats(const Stats& s) {
    require(s.maxHitpoints > 0 && s.maxHitpoints <= 100000000 && s.hitpoints <= s.maxHitpoints &&
            s.hitpoints >= -100000000 && s.maxMana >= 0 && s.maxMana <= 100000000 &&
            s.mana >= 0 && s.mana <= s.maxMana && s.attack >= 0 && s.attack <= 100000000 &&
            s.magicAttack >= 0 && s.magicAttack <= 100000000 && s.level > 0 && s.level <= 1000000 &&
            s.expe >= 0 && s.expe <= 1e12f, "Invalid saved stats.");
    for (float n : {s.armor, s.magicArmor, s.critRate, s.critDamage, s.dodgeRate, s.magicAmplifierRate})
        require(std::isfinite(n) && n >= 0 && n <= 1000000, "Invalid saved combat stat.");
}
void validate(const GameSession& s) {
    validateGuildState(s);
    validateQuestState(s);
    PlayerClassCollection classes;
    PlayerRaceDatabase races;
    locationDatabase locations;
    auto identity = [&](const PlayerRace& race, const PlayerClassTemplate& cls) {
        require(std::any_of(races.templates.begin(), races.templates.end(), [&](const auto& r) { return r.name == race.name; }), "Unknown saved race.");
        require(std::any_of(classes.templates.begin(), classes.templates.end(), [&](const auto& c) { return c.name == cls.name; }), "Unknown saved class.");
        validateStats(cls.stats);
    };
    auto location = [&](const std::string& name) {
        return std::any_of(locations.getLocations().begin(), locations.getLocations().end(), [&](const auto& l) { return l.name == name; });
    };
    const auto& p = s.hero;
    validateStats(p.stats);
    identity(p.race, p.playerClass);
    require(!p.name.empty(), "Missing character name.");
    require(p.currentLocation == "Abandoned Cell" || location(p.currentLocation), "Unknown saved location.");
    LocationType expected = Terrain;
    for (const auto& l : locations.getLocations()) if (l.name == p.currentLocation) expected = l.type;
    require(p.currentLocationType == expected, "Invalid saved location type.");
    for (const auto& name : s.travel.discovered) require(location(name), "Unknown discovered route.");
    for (const auto& name : s.travel.marked) require(s.travel.discovered.count(name), "Undiscovered marked route.");
    require(s.actionCounter >= 0 && s.actionCounter < 4 && s.lastWeekPaid >= 0 &&
            s.lastWeekPaid <= p.timeSystem.getTotalWeeks(), "Invalid action or wage state.");
    require(p.progression.level > 0 && p.progression.exp >= 0 && std::isfinite(p.progression.exp) &&
            p.progression.skillPoints >= 0 && std::isfinite(p.progression.experienceRequired) &&
            p.progression.experienceRequired >= 100, "Invalid progression.");
    for (int n : {p.economy.platinum, p.economy.gold, p.economy.silver, p.economy.copper})
        require(n >= 0 && n <= 100000000, "Invalid currency.");
    require(p.equippedWeaponDebuffChance >= 0 && p.equippedWeaponDebuffChance <= 1, "Invalid weapon chance.");
    auto spells = [&](const std::vector<std::string>& names) {
        const auto& db = s.inventory.spellDB.getSpells();
        for (const auto& name : names)
            require(std::any_of(db.begin(), db.end(), [&](const auto& spell) { return spell.spellName == name; }), "Unknown saved spell.");
    };
    spells(p.learnedSpells); spells(p.equippedStaffSpells);
    require(s.party.size() <= 4, "Too many companions.");
    for (const auto& npc : s.party) {
        validateStats(npc.stats); identity(npc.race, npc.playerClass); spells(npc.spells);
        require(!npc.name.empty() && npc.level > 0 &&
                npc.wagePerWeek >= 0 && npc.wagePerWeek <= 100000000, "Invalid companion.");
    }
    const auto& inv = s.inventory;
    for (const auto& item : inv.inventory) {
        size_t size = 0;
        switch (item.type) {
            case PlayerInventory::ItemType::Potion: size = inv.potionDB.getPotions().size(); break;
            case PlayerInventory::ItemType::Equipment: size = inv.equipmentDB.getEquipment().size(); break;
            case PlayerInventory::ItemType::FoodAndDrink: size = inv.foodDB.getFoodAndDrink().size(); break;
            default: throw std::runtime_error("Unknown item type.");
        }
        require(item.dbIndex < size && item.quantity > 0 && item.quantity <= 1000000, "Invalid inventory item.");
    }
    for (int index : {inv.equipped.armorIndex, inv.equipped.weaponIndex, inv.equipped.staffIndex}) {
        require(index >= -1, "Invalid equipment slot.");
        if (index == -1) continue;
        require(std::any_of(inv.inventory.begin(), inv.inventory.end(), [&](const auto& item) {
            return item.type == PlayerInventory::ItemType::Equipment && item.dbIndex == static_cast<size_t>(index);
        }), "Equipped item missing from inventory.");
    }
    const auto& equipment = inv.equipmentDB.getEquipment();
    if (inv.equipped.armorIndex >= 0)
        require(equipment[inv.equipped.armorIndex].type == "Armor", "Invalid armor slot.");
    if (inv.equipped.weaponIndex >= 0)
        require(equipment[inv.equipped.weaponIndex].type == "Weapon", "Invalid weapon slot.");
    if (inv.equipped.staffIndex >= 0)
        require(equipment[inv.equipped.staffIndex].type == "Staff", "Invalid staff slot.");
    if (inv.activeBuff) {
        const auto& b = *inv.activeBuff;
        require(b.remainingTurns > 0 && b.remainingTurns <= 1000000, "Invalid buff duration.");
        for (int n : {b.attackBonus, b.defenseBonus, b.magicAttackBonus, b.magicDefenseBonus, b.maxManaBonus})
            require(n >= 0 && n <= 1000000, "Invalid buff bonus.");
        require(b.attackBonus <= p.stats.attack && b.magicAttackBonus <= p.stats.magicAttack &&
                b.defenseBonus <= p.stats.armor && b.magicDefenseBonus <= p.stats.magicArmor &&
                b.maxManaBonus <= p.stats.maxMana && b.critRateBonus >= 0 && b.critRateBonus <= p.stats.critRate &&
                b.critDamageBonus >= 0 && b.critDamageBonus <= p.stats.critDamage, "Invalid active buff.");
    }
}
}

namespace saves {
bool save(const GameSession& session, const std::filesystem::path& path, std::string& error) {
    error.clear();
    auto temporary = path; temporary += ".tmp";
    try {
        validate(session);
        std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
        require(static_cast<bool>(out), "Cannot open temporary save file.");
        out.imbue(std::locale::classic());
        out << "RPGGAME 3\n" << std::setprecision(std::numeric_limits<float>::max_digits10);
        auto copy = session;
        Writer writer{out}; writer(copy);
        require(out.tellp() <= 4 * 1024 * 1024, "Save file is too large.");
        out.flush();
        require(static_cast<bool>(out), "Cannot write save file.");
        out.close();
        require(static_cast<bool>(out), "Cannot close save file.");
#ifdef _WIN32
        require(MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH), "Cannot replace save file.");
#else
        std::filesystem::rename(temporary, path);
#endif
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        return false;
    }
}

std::optional<GameSession> load(const std::filesystem::path& path, std::string& error) {
    error.clear();
    try {
        std::ifstream in(path, std::ios::binary);
        require(static_cast<bool>(in), "No readable save found at the save path.");
        require(std::filesystem::file_size(path) <= 4 * 1024 * 1024, "Save file is too large.");
        in.imbue(std::locale::classic());
        std::string header;
        std::getline(in, header);
        require(header == "RPGGAME 1" || header == "RPGGAME 2" || header == "RPGGAME 3", "Unsupported save format or version.");
        PlayerClassCollection classes;
        GameSession result(Player("", PlayerRace{}, classes.templates.front()));
        Reader reader{in, header.back() - '0'}; reader(result);
        in >> std::ws;
        require(in.eof(), "Unexpected trailing save data.");
        validate(result);
        return result;
    } catch (const std::exception& e) {
        error = e.what();
        return std::nullopt;
    }
}
}
