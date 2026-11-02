#include "commissions.hpp"
#include "catalogs/content.hpp"

const std::vector<QuestDefinition>& commissions() {
    return content::catalog<QuestDefinition>("narrative/commissions.json");
}

