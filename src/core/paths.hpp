#pragma once

#include <filesystem>
#include <string>

// Locates the JSON content directory. A single binary cannot carry the data
// files, so release builds ship `data/` next to the executable.
namespace paths {

// Explicit RPG_DATA_DIR environment override (even if missing), then `data`
// under the working directory, then the build-time source directory.
std::filesystem::path dataDir();

// dataDir() / name. Does not check existence.
std::filesystem::path dataFile(const std::string& name);

} // namespace paths
