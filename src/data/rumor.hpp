#pragma once

#include <string>
#include <vector>

struct RumorDefinition {
    std::string id;
    std::string city; // Empty for a destination clue available in any other town/city.
    std::string destination; // Empty for local guild/quest hints.
    std::string text;
};

class RumorDatabase {
public:
    RumorDatabase();
    const std::vector<RumorDefinition>& getRumors() const { return rumors; }
    const RumorDefinition* find(const std::string& id) const;
private:
    std::vector<RumorDefinition> rumors;
};

const RumorDatabase& rumorDatabase();
