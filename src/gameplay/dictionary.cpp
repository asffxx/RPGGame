#include "dictionary.hpp"

#include "core/types.hpp"
#include "core/ui.hpp"
#include "catalogs/debuff.hpp"
#include "catalogs/equipment.hpp"
#include "catalogs/event.hpp"
#include "catalogs/location.hpp"
#include "entities/enemy.hpp"
#include "app/display.hpp"

void showDictionary(Player& hero) {
    hero.hasNewDictionaryEntry = false;
    bool inDictionary = true;
    while (inDictionary) {
        clearScreen();
        std::vector<std::string> sections = {"1. Enemies", "2. Weapons", "3. Locations", "4. Events", "5. Special Characters", "6. Exit"};
        displayBorderedMenu(sections, "Choose a section: ");
        int choice = getNumberInput(1, 6);
        if (choice == 6) {
            inDictionary = false;
            continue;
        }
        if (choice == 1) {
            if (hero.defeatedEnemies.empty()) {
                display::dictionaryEmptyEnemies();
                continue;
            }
            std::vector<std::string> enemyNames(hero.defeatedEnemies.begin(), hero.defeatedEnemies.end());
            PagedSelector enemySelector(enemyNames, 5, "Enemies");
            int idx = enemySelector.select();
            if (idx < 0) continue;
            std::string enemyName = enemyNames[idx];
            EnemyDatabase enemyDB;
            for (const auto& tmpl : enemyDB.templates) {
                if (tmpl.name == enemyName) {
                    bool inEnemy = true;
                    while (inEnemy) {
                        clearScreen();
                        display::dictionaryEnemyHeader(enemyName);
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            display::dictionaryEnemyStats(tmpl);
                        } else if (subChoice == 2) {
                            display::dictionaryEnemyDescription(tmpl.lore.desc);
                        } else {
                            inEnemy = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 2) {
            if (hero.boughtWeapons.empty()) {
                display::dictionaryEmptyWeapons();
                continue;
            }
            std::vector<std::string> weaponNames(hero.boughtWeapons.begin(), hero.boughtWeapons.end());
            PagedSelector weaponSelector(weaponNames, 5, "Weapons");
            int idx = weaponSelector.select();
            if (idx < 0) continue;
            std::string weaponName = weaponNames[idx];
            EquipmentandWeaponDatabase eqDB;
            for (const auto& eq : eqDB.getEquipment()) {
                if (eq.name == weaponName) {
                    bool inWeapon = true;
                    while (inWeapon) {
                        clearScreen();
                        display::dictionaryWeaponHeader(weaponName);
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            display::dictionaryWeaponStats(eq);
                        } else if (subChoice == 2) {
                            display::dictionaryWeaponDescription(eq.effectDesc);
                        } else {
                            inWeapon = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 3) {
            if (hero.discoveredLocations.empty()) {
                display::dictionaryEmptyLocations();
                continue;
            }
            std::vector<std::string> locationNames(hero.discoveredLocations.begin(), hero.discoveredLocations.end());
            PagedSelector locationSelector(locationNames, 5, "Locations");
            int idx = locationSelector.select();
            if (idx < 0) continue;
            std::string locationName = locationNames[idx];
            locationDatabase locDB;
            for (const auto& loc : locDB.getLocations()) {
                if (loc.name == locationName) {
                    bool inLocation = true;
                    while (inLocation) {
                        clearScreen();
                        display::dictionaryLocationHeader(locationName);
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            display::dictionaryLocationStats(loc);
                        } else if (subChoice == 2) {
                            display::dictionaryLocationDescription(loc.description);
                        } else {
                            inLocation = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 4) {
            if (hero.encounteredEvents.empty()) {
                display::dictionaryEmptyEvents();
                continue;
            }
            std::vector<std::string> eventNames(hero.encounteredEvents.begin(), hero.encounteredEvents.end());
            PagedSelector eventSelector(eventNames, 5, "Events");
            int idx = eventSelector.select();
            if (idx < 0) continue;
            std::string eventName = eventNames[idx];
            eventDatabase eventDB;
            for (const auto& ev : eventDB.getEvents()) {
                if (ev.name == eventName) {
                    bool inEvent = true;
                    while (inEvent) {
                        clearScreen();
                        display::dictionaryEventHeader(eventName);
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            display::dictionaryEventStats(ev);
                        } else if (subChoice == 2) {
                            display::dictionaryEventDescription(ev.description);
                        } else {
                            inEvent = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 5) {
            if (hero.hiredSpecialCharacters.empty()) {
                display::dictionaryEmptyCharacters();
                continue;
            }
            clearScreen();
            std::vector<std::string> names(hero.hiredSpecialCharacters.begin(), hero.hiredSpecialCharacters.end());
            display::dictionarySpecialCharacters(names);
        }
    }
}
