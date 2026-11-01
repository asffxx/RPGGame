#include <filesystem>
#include <fstream>
#include <iostream>
#include "catalog_snapshot.hpp"
#include "world/sidequests.hpp"
#include "world/commissions.hpp"
#include "world/quest.hpp"

template<class T> void emit(const std::filesystem::path& directory, const std::string& file, const std::vector<T>& records) {
    std::filesystem::create_directories((directory / file).parent_path());
    std::ofstream out(directory / file);
    out << snapshot::json(records);
    if (!out) throw std::runtime_error("Cannot write snapshot " + file);
    out.close();
    auto decoded = content::readCatalog<T>((directory / file).string(), records.empty());
    if (snapshot::json(records) != snapshot::json(decoded))
        throw std::runtime_error("JSON round-trip mismatch: " + file);
    if (std::filesystem::exists(paths::dataFile(file))) {
        auto source = content::readCatalog<T>(file, records.empty());
        if (snapshot::json(records) != snapshot::json(source))
            throw std::runtime_error("Catalog differs from JSON: " + file);
    }
}
int main(int argc, char** argv) {
    try {
        if (argc != 2) throw std::runtime_error("Pass snapshot directory");
        std::filesystem::path dir(argv[1]);
        std::filesystem::create_directories(dir);
        emit(dir, "items/equipment.json", EquipmentandWeaponDatabase().getEquipment());
        emit(dir, "items/potions.json", PotionDatabase().getPotions());
        emit(dir, "items/food.json", FoodandDrinksDatabase().getFoodAndDrink());
        emit(dir, "items/spells.json", SpellDatabase().getSpells());
        emit(dir, "characters/classes.json", PlayerClassCollection().templates);
        emit(dir, "characters/races.json", PlayerRaceDatabase().templates);
        emit(dir, "characters/skills.json", ClassSkillDatabase().getSkills());
        emit(dir, "world/locations.json", locationDatabase().getLocations());
        emit(dir, "world/enemies.json", EnemyDatabase().templates);
        emit(dir, "world/debuffs.json", DebuffDatabase().getDebuffs());
        emit(dir, "world/events.json", eventDatabase().getEvents());
        emit(dir, "narrative/guilds.json", GuildDatabase().getGuilds());
        emit(dir, "narrative/rumors.json", RumorDatabase().getRumors());
        emit(dir, "narrative/sidequests.json", sideQuests());
        emit(dir, "narrative/commissions.json", commissions());
        emit(dir, "narrative/mainquests.json", mainQuests());
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
