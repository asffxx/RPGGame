#pragma once

#include <string>
#include <vector>

#include "core/types.hpp"
#include "data/equipment.hpp"
#include "data/event.hpp"
#include "data/location.hpp"
#include "entities/enemy.hpp"
#include "entities/npc.hpp"
#include "entities/player.hpp"
#include "systems/inventory.hpp"

// Centralized presentation layer. Every std::cout in the project lives here so
// the gameplay/logic modules stay free of output concerns. Functions are grouped
// by the module that owns the corresponding logic.

namespace display {

// --- core/ui (shared prompts) ---
void pressEnter();
void invalidChoice();

// --- core/timesystem ---
void sleepPenalty();

// --- entities/enemy ---
void enemyAppeared(const Enemy& enemy);
void enemyDefeated(const Enemy& enemy, const Player& player);

// --- combat ---
void combatRunAway();
void combatInvalidChoice();
void combatSpellPrompt(const std::vector<std::string>& spellNames);
void combatManaInsufficient();
void combatVictory(const std::string& enemyName);
void combatDefeat(const std::string& playerName);

// --- systems/inventory ---
void inventoryEmpty();
void buffWoreOff(const std::string& name);
bool usePotionPrompt(const std::string& potionName);
void potionRecoveredHp(int heal);
void potionRecoveredMana(int restore);
void potionEffect(const std::string& effectDesc);
bool useFoodPrompt(const std::string& foodName);
void foodRecoveredHp(int heal);
void foodRecoveredMana(int restore);
void foodMaxHpIncrease(int amount);
void foodMaxManaIncrease(int amount);
bool unequipPrompt(const std::string& itemName);
void itemUnequipped(const std::string& itemName);
bool equipPrompt(const std::string& itemName);
void itemEffect(const std::string& effectDesc);

// --- systems/event ---
void eventHeader(const std::string& name, const std::string& description);
void eventGoldGained(int amount);
void eventGoldLost(int amount);
void eventNotEnoughGold();
void eventExpGained(int amount);
void eventExpLost(int amount);
void eventHpRecovered(int heal);
void eventHpDamage(int damage, bool survived);
void eventManaRecovered(int restore);
void eventManaLost(int drain);
void eventEnemyEncounter();
void eventPressEnter();

// --- systems/party ---
void wagesPaid(int total);
void wagesInsufficient();
void partyEmpty();
void partyMemberLeft(const std::string& name);
void partyMemberList(const std::vector<NPC>& party);
void partyActionPrompt(const std::string& name);
void partyDialogue(const std::string& name, const std::string& line);
void partyKicked(const std::string& name);
void partyStory(const std::string& name, const std::string& story);

// --- systems/dictionary ---
void dictionaryEmptyEnemies();
void dictionaryEmptyWeapons();
void dictionaryEmptyLocations();
void dictionaryEmptyEvents();
void dictionaryEmptyCharacters();
void dictionaryEnemyHeader(const std::string& name);
void dictionaryEnemyStats(const EnemyDatabase::EnemyTemplate& tmpl);
void dictionaryEnemyDescription(const std::string& desc);
void dictionaryWeaponHeader(const std::string& name);
void dictionaryWeaponStats(const EquipmentandWeaponDatabase::equipmentProperties& eq);
void dictionaryWeaponDescription(const std::string& desc);
void dictionaryLocationHeader(const std::string& name);
void dictionaryLocationStats(const locationDatabase::locationProperties& loc);
void dictionaryLocationDescription(const std::string& desc);
void dictionaryEventHeader(const std::string& name);
void dictionaryEventStats(const eventDatabase::eventProperties& ev);
void dictionaryEventDescription(const std::string& desc);
void dictionarySpecialCharacters(const std::vector<std::string>& names);

// --- ui/menu ---
void menuInvalidChoice();

// --- ui/player_controller ---
void showPlayerStats(const Player& player, float reqAmount, const std::optional<PlayerInventory::Buff>& activeBuff);
void leveledUp();

// --- world/establishments ---
void tavernHeader();
void tavernSlept();
void tavernNoCoin();
void tavernExit();
void tavernFoodPurchased(const std::string& name);
void tavernFoodTooExpensive();
void tavernPartyTooBig();
void tavernHireInfo(const NPC& npc, int totalCost);
void tavernHireDeclined();
void tavernHireTooExpensive();
void tavernHireJoined(const NPC& npc, const std::string& line);
void storeHeader(const Player& player);
void storeBoughtPotion(const std::string& name);
void storeBoughtEquipment(const std::string& name);
void storeNotEnoughCurrency();
void magicStoreHeader(const Player& player);
void magicStoreNoSpells();
void magicStoreLearned(const std::string& name);
void magicStoreAlreadyKnown(const std::string& name);
void magicStoreNoEnchantable();
void magicStoreEnchantItemHeader();
void magicStoreEnchantChoice(const std::vector<std::string>& lines);
void magicStoreNotEnoughGold();
void magicStoreEnchanted(const std::string& itemName, const std::string& applied);

// --- world/travel ---
void travelNoMarkedLocations();
void travelLocationList(const std::vector<std::string>& lines);
void travelDiscovered(const std::string& name);
void travelLocationHeader(const std::string& name, const std::string& description);
void travelLocationMarked();

// --- app/main_menu ---
void timePassed();
void gameBasics();
void characterCreated();
void stepIntoGrey();

// --- main.cpp ---
void acceptFatePrompt();

} // namespace display
