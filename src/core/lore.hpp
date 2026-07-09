#pragma once

#include <string>

// Generic descriptive lore attached to a class or other entity.
struct Lore {
    std::string description;
    Lore(const std::string& desc = "") : description(desc) {}
};

// Lore attached to a player race template.
struct RaceLore {
    std::string description;
};
