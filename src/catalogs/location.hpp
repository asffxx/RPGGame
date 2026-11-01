#pragma once

#include <string>
#include <vector>

#include "core/types.hpp"

// Static catalog of world locations.
class locationDatabase {
public:
    struct locationProperties {
        std::string name;
        std::string description;
        int difficultyLevel;
        LocationType type;
        std::string id{};
    };

    locationDatabase();

    const std::vector<locationProperties>& getLocations() const {
        return locationList;
    }

    const locationProperties* find(const std::string& id) const;

private:
    std::vector<locationProperties> locationList;
};
