#pragma once

#include <set>
#include "quest_types.hpp"

struct GameSession;
class EnemyController;
class CombatSystem;
class NPCGenerator;

const char* questCategoryName(QuestCategory category);
const std::vector<QuestDefinition>& allQuests();
const QuestDefinition* findQuest(const std::string& id);
QuestProgress questProgress(const GameSession& session, const std::string& id);
std::vector<const QuestDefinition*> boardQuests(const GameSession& session, QuestCategory category);
void discoverBoardQuests(GameSession& session, QuestCategory category);
std::string questLockReason(const GameSession& session, const QuestDefinition& quest);
bool acceptQuest(GameSession& session, const std::string& id);
bool completeQuestObjective(GameSession& session, const std::string& id, bool victorious);
bool claimQuestReward(GameSession& session, const std::string& id);
std::vector<std::string> questsAtLocation(const GameSession& session);
std::set<std::string> travelLeads(const GameSession& session);
void validateQuestState(const GameSession& session);
void openQuestBoard(GameSession& session, QuestCategory category);
void showQuestJournal(const GameSession& session);
void performQuestAction(GameSession& session, EnemyController& enemies, CombatSystem& combat, NPCGenerator& generator);
