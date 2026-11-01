#pragma once

#include <set>
#include <string>

struct TravelState {
    std::set<std::string> discovered;
    std::set<std::string> marked;
};
