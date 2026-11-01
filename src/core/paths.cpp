#include "paths.hpp"

#include <cstdlib>

namespace paths {

std::filesystem::path dataDir() {
    std::error_code ignored;
    if (const char* env = std::getenv("RPG_CONTENT_DIR")) {
        std::filesystem::path candidate(env);
        return candidate;
    }
    if (std::filesystem::is_directory(local, ignored)) return local;
#ifdef RPG_CONTENT_DIR
    {
        std::filesystem::path candidate(RPG_CONTENT_DIR);
        if (std::filesystem::is_directory(candidate, ignored)) return candidate;
    }
#endif
    return local;
}

std::filesystem::path dataFile(const std::string& name) {
    return dataDir() / name;
}

} // namespace paths
