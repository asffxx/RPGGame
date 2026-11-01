#pragma once

#include <iomanip>
#include <locale>
#include <sstream>
#include "catalogs/content.hpp"

// Verification/export only: not linked into the game. Float output preserves
// the exact original float value, not a rounded balance approximation.
namespace snapshot {
inline void quote(std::ostream& out, const std::string& s) {
    out << '"';
    for (unsigned char c : s) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 32) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c) << std::dec;
        else out << c;
    }
    out << '"';
}
struct Writer {
    std::ostream& out;
    bool first = true;
    template<class T> void operator()(const char* name, const T& v) {
        if (!first) out << ',';
        first = false;
        quote(out, name); out << ':'; value(v);
    }
    template<class T> void value(const T& v) {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>) quote(out, v);
        else if constexpr (std::is_same_v<T, bool>) out << (v ? "true" : "false");
        else if constexpr (std::is_enum_v<T>) out << static_cast<int>(v);
        else if constexpr (std::is_arithmetic_v<T>) out << std::setprecision(std::numeric_limits<T>::max_digits10) << v;
        else { out << '{'; Writer nested{out}; auto copy = v; content::fields(nested, copy); out << '}'; }
    }
    template<class T> void value(const std::vector<T>& v) {
        out << '[';
        for (size_t i = 0; i < v.size(); ++i) { if (i) out << ','; value(v[i]); }
        out << ']';
    }
};
template<class T> std::string json(const std::vector<T>& records) {
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << "{\"schemaVersion\":1,\"records\":";
    Writer{out}.value(records);
    out << "}\n";
    return out.str();
}
}
