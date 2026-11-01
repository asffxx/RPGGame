#include "ids.hpp"

#include <cctype>

namespace ids {

std::string slug(const std::string& text) {
    std::string result;
    bool pendingUnderscore = false;
    for (unsigned char c : text) {
        if (std::isalnum(c)) {
            if (pendingUnderscore && !result.empty()) result.push_back('_');
            pendingUnderscore = false;
            result.push_back(static_cast<char>(std::tolower(c)));
        } else {
            pendingUnderscore = true;
        }
    }
    if (result.empty()) result = "item";
    if (std::isdigit(static_cast<unsigned char>(result.front()))) result = "n_" + result;
    return result;
}

std::string unique(const std::string& text, std::set<std::string>& used) {
    auto base = slug(text);
    auto candidate = base;
    for (int suffix = 2; used.count(candidate) != 0; ++suffix) {
        candidate = base + "_" + std::to_string(suffix);
    }
    used.insert(candidate);
    return candidate;
}

bool valid(const std::string& id) {
    if (id.empty()) return false;
    if (!std::isalpha(static_cast<unsigned char>(id.front()))) return false;
    for (unsigned char c : id) {
        if (c != '_' && !std::isalnum(c)) return false;
        if (std::isupper(c)) return false;
    }
    return id.back() != '_';
}

} // namespace ids
