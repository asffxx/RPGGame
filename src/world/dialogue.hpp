#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "world/conditions.hpp"

struct GameSession;

struct DialogueChoice {
    std::string id;
    std::string text;
    std::string next;
    std::string result;
    bool once = false;
    std::vector<ConditionRule> conditions;
    std::vector<ConditionRule> effects;
};

struct DialogueNode {
    std::string id;
    std::string text;
    std::vector<DialogueChoice> choices;
};

struct DialogueResident {
    std::string id;
    std::string name;
    std::string city;
    std::string role;
    std::string concern;
    std::string concernFlag;
    std::string entry;
    std::vector<DialogueNode> nodes;
    const DialogueNode* node(const std::string& id) const;
};

class DialogueDatabase {
public:
    bool load(const std::filesystem::path& root, std::string& error);
    bool loaded() const { return loaded_; }
    const std::string& error() const { return error_; }
    const std::vector<DialogueResident>& residents() const { return residents_; }
    const DialogueResident* find(const std::string& id) const;
private:
    bool loaded_ = false;
    std::string error_;
    std::vector<DialogueResident> residents_;
};

const DialogueDatabase& dialogueDatabase();
std::string dialogueLockReason(const GameSession& session, const DialogueResident& npc,
                               const DialogueChoice& choice);
bool chooseDialogue(GameSession& session, const DialogueResident& npc,
                    const DialogueChoice& choice, std::string& error);
void openDialogue(GameSession& session, const DialogueResident& npc);
void openResidents(GameSession& session);
