#pragma once

#include <string>
#include <vector>

#include "core/types.hpp"
#include "catalogs/equipment.hpp"
#include "catalogs/event.hpp"
#include "catalogs/location.hpp"
#include "entities/enemy.hpp"
#include "entities/npc.hpp"
#include "entities/player.hpp"
#include "gameplay/inventory.hpp"

// Centralized presentation layer. Every std::cout in the project lives here so
// the gameplay/logic modules stay free of output concerns. Functions are grouped
// by the module that owns the corresponding logic.

namespace display {

// --- core/ui (shared prompts) ---
void pressEnter();

// --- core/timesystem ---
void sleepPenalty();

// --- entities/enemy ---
void enemyAppeared(const Enemy& enemy);
// Compact coin amount, e.g. "5g 70c"; "0c" when empty.
std::string coins(int platinum, int gold, int silver, int copper);
// Boxed victory/spoils screen with a pause to read it.
void spoils(const std::vector<std::string>& lines);

// Formatted values (two decimals, trailing zeros trimmed).
std::string num(float value);

// --- combat ---
void combatRunAway();
void combatInvalidChoice();
void combatSpellPrompt(const std::vector<std::string>& spellNames);
void combatManaInsufficient();
void combatDefeat(const std::string& playerName);

// --- systems/inventory ---
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
void eventBox(const std::vector<std::string>& lines);
void eventPressEnter();

// --- systems/party ---
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

// --- ui/player_controller ---
void showPlayerStats(const Player& player, float reqAmount, const std::optional<PlayerInventory::Buff>& activeBuff);
void leveledUp();

// --- world/establishments ---
void tavernFoodPurchased(const std::string& name);
void tavernFoodTooExpensive();
void tavernPartyTooBig();
void tavernHireDeclined();
void tavernHireTooExpensive();
void tavernHireJoined(const NPC& npc, const std::string& line);
void storeBoughtPotion(const std::string& name);
void storeBoughtEquipment(const std::string& name);
void storeNotEnoughCurrency();
void storeSold(const std::string& name, int quantity, long long copper);
void storeNothingToSell();
void magicStoreNoSpells();
void magicStoreLearned(const std::string& name);
void magicStoreAlreadyKnown(const std::string& name);
void magicStoreNoEnchantable();
void magicStoreEnchantChoice(const std::vector<std::string>& lines);
void magicStoreNotEnoughGold();
void magicStoreEnchanted(const std::string& itemName, const std::string& applied);

// --- world/travel ---
void travelNoMarkedLocations();
void travelLocationList(const std::vector<std::string>& lines);
void travelLocationHeader(const std::string& name, const std::string& description, bool hasQuest = false);
void travelLocationMarked();

// --- app/main_menu ---
void gameBasics();
void message(const std::string& text);
void message(const std::vector<std::string>& lines);
void racePreview(const PlayerRaceTemplate& race);
void classPreview(const PlayerClassTemplate& playerClass, const PlayerRaceTemplate& race);
std::string characterName();

} // namespace display
