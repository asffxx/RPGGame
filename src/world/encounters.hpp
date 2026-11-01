#pragma once

#include "data/location.hpp"

locationDatabase::locationProperties currentArea(const std::string& name);
float encounterChance(LocationType type);
