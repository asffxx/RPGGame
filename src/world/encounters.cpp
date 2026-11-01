#include "encounters.hpp"

locationDatabase::locationProperties currentArea(const std::string& name) {
    static const locationDatabase database;
    for (const auto& area : database.getLocations()) if (area.name == name) return area;
    return {"Abandoned Cell", "The cell where your journey began.", 1, Terrain};
}

float encounterChance(LocationType type) {
    switch (type) {
        case PeacefulVillage: return 0.2f;
        case PeacefulTown: return 0.3f;
        case Dungeon: return 0.8f;
        case Terrain: return 0.6f;
        case SpellStore: return 0.5f;
        case PeacefulCity: return 0.0f;
    }
    return 0.0f;
}
