#pragma once

#include "catalogs/location.hpp"

locationDatabase::locationProperties currentArea(const std::string& name);
float encounterChance(LocationType type);
