#include "dictionary.hpp"

#include <iostream>

#include "core/types.hpp"
#include "core/ui.hpp"
#include "db/debuff.hpp"
#include "db/equipment.hpp"
#include "db/event.hpp"
#include "db/location.hpp"
#include "game/entities/enemy.hpp"

void showDictionary(Player& hero) {
    hero.hasNewDictionaryEntry = false;
    bool inDictionary = true;
    while (inDictionary) {
        system("cls");
        std::vector<std::string> sections = {"1. Enemies", "2. Weapons", "3. Locations", "4. Events", "5. Special Characters", "6. Exit"};
        displayBorderedMenu(sections, "Choose a section: ");
        int choice = getNumberInput(1, 6);
        if (choice == 6) {
            inDictionary = false;
            continue;
        }
        if (choice == 1) {
            if (hero.defeatedEnemies.empty()) {
                std::cout << "No defeated enemies yet.\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::vector<std::string> enemyNames(hero.defeatedEnemies.begin(), hero.defeatedEnemies.end());
            PagedSelector enemySelector(enemyNames);
            size_t idx = enemySelector.select();
            std::string enemyName = enemyNames[idx];
            EnemyDatabase enemyDB;
            for (const auto& tmpl : enemyDB.templates) {
                if (tmpl.name == enemyName) {
                    bool inEnemy = true;
                    while (inEnemy) {
                        system("cls");
                        std::cout << "=== " << enemyName << " ===\n";
                        std::cout << "1. Stats\n";
                        std::cout << "2. Description\n";
                        std::cout << "3. Exit\n";
                        std::cout << "Choose: ";
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            std::cout << "HP: " << tmpl.stats.data.hitpoints << "/" << tmpl.stats.data.maxHitpoints << "\n";
                            std::cout << "Attack: " << tmpl.stats.data.attack << "\n";
                            std::cout << "Magic Attack: " << tmpl.stats.data.magicAttack << "\n";
                            std::cout << "Armor: " << tmpl.stats.data.armor << "\n";
                            std::cout << "Magic Armor: " << tmpl.stats.data.magicArmor << "\n";
                            std::cout << "Level: " << tmpl.stats.data.level << "\n";
                            std::cout << "Experience: " << tmpl.stats.data.expe << "\n";
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else if (subChoice == 2) {
                            std::cout << tmpl.lore.desc << "\n";
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else {
                            inEnemy = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 2) {
            if (hero.boughtWeapons.empty()) {
                std::cout << "No bought weapons yet.\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::vector<std::string> weaponNames(hero.boughtWeapons.begin(), hero.boughtWeapons.end());
            PagedSelector weaponSelector(weaponNames);
            size_t idx = weaponSelector.select();
            std::string weaponName = weaponNames[idx];
            EquipmentandWeaponDatabase eqDB;
            for (const auto& eq : eqDB.getEquipment()) {
                if (eq.name == weaponName) {
                    bool inWeapon = true;
                    while (inWeapon) {
                        system("cls");
                        std::cout << "=== " << weaponName << " ===\n";
                        std::cout << "1. Stats\n";
                        std::cout << "2. Description\n";
                        std::cout << "3. Exit\n";
                        std::cout << "Choose: ";
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            std::cout << "Type: " << eq.type << "\n";
                            std::cout << "Attack Increase: " << eq.attackIncrease << "\n";
                            std::cout << "Magic Attack Increase: " << eq.magicAttackIncrease << "\n";
                            std::cout << "Crit Rate Increase: " << eq.critRateIncrease << "\n";
                            std::cout << "Crit Damage Increase: " << eq.critDamageIncrease << "\n";
                            std::cout << "Price: " << eq.pricePlatinum << "p " << eq.priceGold << "g " << eq.priceSilver << "s " << eq.priceCopper << "c\n";
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else if (subChoice == 2) {
                            std::cout << eq.effectDesc << "\n";
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else {
                            inWeapon = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 3) {
            if (hero.discoveredLocations.empty()) {
                std::cout << "No discovered locations yet.\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::vector<std::string> locationNames(hero.discoveredLocations.begin(), hero.discoveredLocations.end());
            PagedSelector locationSelector(locationNames);
            size_t idx = locationSelector.select();
            std::string locationName = locationNames[idx];
            locationDatabase locDB;
            for (const auto& loc : locDB.getLocations()) {
                if (loc.name == locationName) {
                    bool inLocation = true;
                    while (inLocation) {
                        system("cls");
                        std::cout << "=== " << locationName << " ===\n";
                        std::cout << "1. Stats\n";
                        std::cout << "2. Description\n";
                        std::cout << "3. Exit\n";
                        std::cout << "Choose: ";
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            std::cout << "Difficulty Level: " << loc.difficultyLevel << "\n";
                            std::string typeStr;
                            switch (loc.type) {
                                case PeacefulVillage: typeStr = "Peaceful Village"; break;
                                case PeacefulTown: typeStr = "Peaceful Town"; break;
                                case Dungeon: typeStr = "Dungeon"; break;
                                case Terrain: typeStr = "Terrain"; break;
                                case SpellStore: typeStr = "Spell Store"; break;
                            }
                            std::cout << "Type: " << typeStr << "\n";
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else if (subChoice == 2) {
                            std::cout << loc.description << "\n";
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else {
                            inLocation = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 4) {
            if (hero.encounteredEvents.empty()) {
                std::cout << "No encountered events yet.\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::vector<std::string> eventNames(hero.encounteredEvents.begin(), hero.encounteredEvents.end());
            PagedSelector eventSelector(eventNames);
            size_t idx = eventSelector.select();
            std::string eventName = eventNames[idx];
            eventDatabase eventDB;
            for (const auto& ev : eventDB.getEvents()) {
                if (ev.name == eventName) {
                    bool inEvent = true;
                    while (inEvent) {
                        system("cls");
                        std::cout << "=== " << eventName << " ===\n";
                        std::cout << "1. Stats\n";
                        std::cout << "2. Description\n";
                        std::cout << "3. Exit\n";
                        std::cout << "Choose: ";
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            std::cout << "Gold Reward: " << ev.goldReward << "\n";
                            std::cout << "Exp Reward: " << ev.expReward << "\n";
                            std::cout << "Health Effect: " << ev.healthEffect << "\n";
                            std::cout << "Mana Effect: " << ev.manaEffect << "\n";
                            std::cout << "Enemy Encounter: " << (ev.enemyEncounter ? "Yes" : "No") << "\n";
                            if (ev.enemyEncounter) {
                                std::cout << "Enemy: " << ev.enemyName << "\n";
                                std::cout << "Count: " << ev.enemyCount << "\n";
                            }
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else if (subChoice == 2) {
                            std::cout << ev.description << "\n";
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else {
                            inEvent = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 5) {
            if (hero.hiredSpecialCharacters.empty()) {
                std::cout << "No hired special characters yet.\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            system("cls");
            std::cout << "=== Special Characters ===\n";
            for (const auto& name : hero.hiredSpecialCharacters) {
                std::cout << "- " << name << "\n";
            }
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}
