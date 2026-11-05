#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include "app/session.hpp"

namespace saves {
inline constexpr const char* defaultPath = "savegame.dat";
inline constexpr int saveFormat = 5;

bool save(const GameSession& session, const std::filesystem::path& path, std::string& error);

// Writes an older format. Used by migration tests; production code calls save.
bool saveWithFormat(const GameSession& session, const std::filesystem::path& path, int format,
                    std::string& error);

std::optional<GameSession> load(const std::filesystem::path& path, std::string& error);
}
