#include "datafile.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace datafile {

bool Value::has(const std::string& key) const {
    return type_ == Type::Object && object_.count(key) != 0;
}

const Value& Value::at(const std::string& key) const {
    if (type_ != Type::Object) throw std::runtime_error("Value is not an object.");
    auto it = object_.find(key);
    if (it == object_.end()) throw std::runtime_error("Missing field \"" + key + "\".");
    return it->second;
}

const Value& Value::at(std::size_t index) const {
    if (type_ != Type::Array) throw std::runtime_error("Value is not an array.");
    if (index >= array_.size()) throw std::runtime_error("Array index out of range.");
    return array_[index];
}

std::size_t Value::size() const {
    if (type_ == Type::Array) return array_.size();
    if (type_ == Type::Object) return object_.size();
    return 0;
}

const std::map<std::string, Value>& Value::object() const {
    if (type_ != Type::Object) throw std::runtime_error("Value is not an object.");
    return object_;
}

const std::vector<Value>& Value::array() const {
    if (type_ != Type::Array) throw std::runtime_error("Value is not an array.");
    return array_;
}

const std::string& Value::string() const {
    if (type_ != Type::String) throw std::runtime_error("Value is not a string.");
    return string_;
}

double Value::number() const {
    if (type_ != Type::Number) throw std::runtime_error("Value is not a number.");
    return number_;
}

int Value::integer() const {
    if (type_ != Type::Number) throw std::runtime_error("Value is not a number.");
    if (!std::isfinite(number_) || number_ != std::floor(number_) ||
        number_ < -2147483648.0 || number_ > 2147483647.0)
        throw std::runtime_error("Value is not a 32-bit integer.");
    return static_cast<int>(number_);
}

bool Value::boolean() const {
    if (type_ != Type::Bool) throw std::runtime_error("Value is not a boolean.");
    return bool_;
}

constexpr int maxDepth = 64;
constexpr std::size_t maxString = 65536;

class Parser {
public:
    explicit Parser(const std::string& text) : text_(text) {}

    bool run(Value& out) {
        skipSpace();
        if (!parseValue(out, 0)) return false;
        skipSpace();
        if (pos_ != text_.size()) return fail("unexpected trailing data");
        return true;
    }

    std::string error;

private:
    const std::string& text_;
    std::size_t pos_ = 0;

    bool fail(const std::string& message) {
        std::size_t line = 1;
        std::size_t column = 1;
        for (std::size_t i = 0; i < pos_ && i < text_.size(); ++i) {
            if (text_[i] == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
        }
        error = "line " + std::to_string(line) + ", column " + std::to_string(column) + ": " + message;
        return false;
    }

    bool eof() const { return pos_ >= text_.size(); }
    char peek() const { return eof() ? '\0' : text_[pos_]; }

    void skipSpace() {
        while (!eof()) {
            char c = text_[pos_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++pos_;
            else break;
        }
    }

    bool expect(char c, const char* what) {
        if (peek() != c) return fail(std::string("expected ") + what);
        ++pos_;
        return true;
    }

    bool parseValue(Value& out, int depth) {
        if (depth > maxDepth) return fail("nesting is too deep");
        skipSpace();
        if (eof()) return fail("unexpected end of file");
        char c = peek();
        if (c == '{') return parseObject(out, depth);
        if (c == '[') return parseArray(out, depth);
        if (c == '"') {
            out.type_ = Value::Type::String;
            return parseString(out.string_);
        }
        if (c == 't' || c == 'f' || c == 'n') return parseLiteral(out);
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber(out);
        return fail("unexpected character");
    }

    bool parseLiteral(Value& out) {
        auto match = [&](const char* word) {
            std::size_t length = std::char_traits<char>::length(word);
            if (text_.compare(pos_, length, word) != 0) return false;
            pos_ += length;
            return true;
        };
        if (peek() == 't') {
            if (!match("true")) return fail("invalid literal");
            out.type_ = Value::Type::Bool;
            out.bool_ = true;
            return true;
        }
        if (peek() == 'f') {
            if (!match("false")) return fail("invalid literal");
            out.type_ = Value::Type::Bool;
            out.bool_ = false;
            return true;
        }
        if (!match("null")) return fail("invalid literal");
        out.type_ = Value::Type::Null;
        return true;
    }

    bool parseNumber(Value& out) {
        std::size_t start = pos_;
        if (peek() == '-') ++pos_;
        if (eof()) return fail("truncated number");
        if (peek() == '0') {
            ++pos_;
            if (!eof() && std::isdigit(static_cast<unsigned char>(peek())))
                return fail("leading zeros are not allowed");
        } else if (peek() >= '1' && peek() <= '9') {
            while (!eof() && std::isdigit(static_cast<unsigned char>(peek()))) ++pos_;
        } else {
            return fail("expected a digit");
        }
        if (peek() == '.') {
            ++pos_;
            if (eof() || !std::isdigit(static_cast<unsigned char>(peek())))
                return fail("expected a digit after the decimal point");
            while (!eof() && std::isdigit(static_cast<unsigned char>(peek()))) ++pos_;
        }
        if (peek() == 'e' || peek() == 'E') {
            ++pos_;
            if (peek() == '+' || peek() == '-') ++pos_;
            if (eof() || !std::isdigit(static_cast<unsigned char>(peek())))
                return fail("expected a digit in the exponent");
            while (!eof() && std::isdigit(static_cast<unsigned char>(peek()))) ++pos_;
        }
        std::string token = text_.substr(start, pos_ - start);
        char* end = nullptr;
        double value = std::strtod(token.c_str(), &end);
        if (!end || *end != '\0' || !std::isfinite(value)) return fail("number is out of range");
        out.type_ = Value::Type::Number;
        out.number_ = value;
        return true;
    }

    static void appendUtf8(std::string& out, unsigned int code) {
        if (code <= 0x7F) {
            out.push_back(static_cast<char>(code));
        } else if (code <= 0x7FF) {
            out.push_back(static_cast<char>(0xC0 | (code >> 6)));
            out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        } else if (code <= 0xFFFF) {
            out.push_back(static_cast<char>(0xE0 | (code >> 12)));
            out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | (code >> 18)));
            out.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        }
    }

    bool parseHex4(unsigned int& value) {
        value = 0;
        for (int i = 0; i < 4; ++i) {
            if (eof()) return fail("truncated unicode escape");
            char c = text_[pos_++];
            value <<= 4;
            if (c >= '0' && c <= '9') value |= static_cast<unsigned int>(c - '0');
            else if (c >= 'a' && c <= 'f') value |= static_cast<unsigned int>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') value |= static_cast<unsigned int>(c - 'A' + 10);
            else return fail("invalid unicode escape");
        }
        return true;
    }

    bool parseString(std::string& out) {
        out.clear();
        if (!expect('"', "an opening quote")) return false;
        while (true) {
            if (eof()) return fail("unterminated string");
            char c = text_[pos_++];
            if (c == '"') return true;
            if (static_cast<unsigned char>(c) < 0x20) return fail("control character in string");
            if (c != '\\') {
                out.push_back(c);
                if (out.size() > maxString) return fail("string is too long");
                continue;
            }
            if (eof()) return fail("unterminated escape");
            char escape = text_[pos_++];
            switch (escape) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'u': {
                    unsigned int code = 0;
                    if (!parseHex4(code)) return false;
                    if (code >= 0xD800 && code <= 0xDBFF) {
                        if (text_.compare(pos_, 2, "\\u") != 0) return fail("missing low surrogate");
                        pos_ += 2;
                        unsigned int low = 0;
                        if (!parseHex4(low)) return false;
                        if (low < 0xDC00 || low > 0xDFFF) return fail("invalid low surrogate");
                        code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                    } else if (code >= 0xDC00 && code <= 0xDFFF) {
                        return fail("unexpected low surrogate");
                    }
                    appendUtf8(out, code);
                    break;
                }
                default: return fail("invalid escape");
            }
            if (out.size() > maxString) return fail("string is too long");
        }
    }

    bool parseObject(Value& out, int depth) {
        out.type_ = Value::Type::Object;
        out.object_.clear();
        if (!expect('{', "'{'")) return false;
        skipSpace();
        if (peek() == '}') {
            ++pos_;
            return true;
        }
        while (true) {
            skipSpace();
            if (peek() != '"') return fail("expected a field name");
            std::string key;
            if (!parseString(key)) return false;
            skipSpace();
            if (!expect(':', "':'")) return false;
            Value entry;
            if (!parseValue(entry, depth + 1)) return false;
            if (!out.object_.emplace(key, std::move(entry)).second)
                return fail("duplicate field \"" + key + "\"");
            skipSpace();
            if (peek() == ',') {
                ++pos_;
                continue;
            }
            return expect('}', "'}'");
        }
    }

    bool parseArray(Value& out, int depth) {
        out.type_ = Value::Type::Array;
        out.array_.clear();
        if (!expect('[', "'['")) return false;
        skipSpace();
        if (peek() == ']') {
            ++pos_;
            return true;
        }
        while (true) {
            Value entry;
            if (!parseValue(entry, depth + 1)) return false;
            out.array_.push_back(std::move(entry));
            skipSpace();
            if (peek() == ',') {
                ++pos_;
                continue;
            }
            return expect(']', "']'");
        }
    }
};

bool parse(const std::string& text, Value& out, std::string& error) {
    error.clear();
    Parser parser(text);
    Value result;
    if (!parser.run(result)) {
        error = parser.error;
        return false;
    }
    out = std::move(result);
    return true;
}

bool loadFile(const std::filesystem::path& path, Value& out, std::string& error) {
    error.clear();
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "cannot open " + path.string();
        return false;
    }
    std::string text{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
    if (text.size() > 4 * 1024 * 1024) {
        error = path.string() + ": file is too large";
        return false;
    }
    if (!parse(text, out, error)) {
        error = path.string() + ": " + error;
        return false;
    }
    return true;
}

} // namespace datafile
