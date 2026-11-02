#include "sidequests.hpp"
#include "catalogs/content.hpp"

const std::vector<QuestDefinition>& sideQuests() {
    return content::catalog<QuestDefinition>("narrative/sidequests.json");
}

