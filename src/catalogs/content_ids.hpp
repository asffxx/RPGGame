#pragma once

#include <set>
#include <string>

// Stable content identifiers. Every catalog record gets one, so saves and the
// pixel-engine data files resolve content by name-free keys instead of table
// positions.
namespace ids {

// Lowercase snake_case form of a display name. Non-alphanumeric runs become a
// single underscore; leading digits get an "n_" prefix so the result is always
// a valid identifier.
std::string slug(const std::string& text);

// Slug of `text`, made unique against `used`. Repeated names get "_2", "_3",
// and so on, in catalog order. The chosen id is inserted into `used`.
std::string unique(const std::string& text, std::set<std::string>& used);

// True for a non-empty lowercase snake_case identifier.
bool valid(const std::string& id);

} // namespace ids
