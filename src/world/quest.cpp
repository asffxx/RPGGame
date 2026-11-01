#include "quest.hpp"

const std::vector<QuestDefinition>& mainQuests() {
    static const std::vector<QuestDefinition> quests;
    return quests;
}
