#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include "app/session.hpp"

namespace saves {
inline constexpr const char* defaultPath = "savegame.dat";
bool save(const GameSession& session, const std::filesystem::path& path, std::string& error);
std::optional<GameSession> load(const std::filesystem::path& path, std::string& error);
}
