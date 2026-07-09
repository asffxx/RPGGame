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
    };

    locationDatabase();

    const std::vector<locationProperties>& getLocations() const {
        return locationList;
    }

private:
    std::vector<locationProperties> locationList;
};
