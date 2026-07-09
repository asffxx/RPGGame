#pragma once

#include <string>
#include <vector>

// Static catalog of random events.
class eventDatabase {
public:
    struct eventProperties {
        std::string name;
        std::string description;
        int goldReward;
        int expReward;
        int healthEffect;
        int manaEffect;
        bool enemyEncounter;
        std::string enemyName;
        int enemyCount;

    };

    eventDatabase();

    const std::vector<eventProperties>& getEvents() const {
        return eventList;
    }

private:
    std::vector<eventProperties> eventList;
};
