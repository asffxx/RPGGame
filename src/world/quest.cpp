#include "quest.hpp"
#include "catalogs/content.hpp"

const std::vector<QuestDefinition>& mainQuests() {
    return content::catalog<QuestDefinition>("narrative/mainquests.json", true);
}

