#include "location.hpp"
#include "content.hpp"

locationDatabase::locationDatabase()
    : locationList(content::catalog<locationProperties>("world/locations.json")) {}

const locationDatabase::locationProperties* locationDatabase::find(const std::string& id) const {
    for (const auto& item : locationList)
        if (item.id == id) return &item;
    return nullptr;
}
