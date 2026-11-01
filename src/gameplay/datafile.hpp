#pragma once

#include <cstddef>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

// Strict reader for the JSON subset used by the data files the pixel engine
// will also read. Supported: objects, arrays, strings, numbers, booleans, and
// null. Rejected on purpose: comments, trailing commas, duplicate keys, lone
// surrogates, and non-finite numbers. Every error names a line and column.
namespace datafile {

class Value {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    Type type() const { return type_; }
    bool isNull() const { return type_ == Type::Null; }
    bool isObject() const { return type_ == Type::Object; }
    bool isArray() const { return type_ == Type::Array; }

    bool has(const std::string& key) const;
    const Value& at(const std::string& key) const;
    const Value& at(std::size_t index) const;

    std::size_t size() const;
    const std::map<std::string, Value>& object() const;
    const std::vector<Value>& array() const;

    const std::string& string() const;
    double number() const;
    int integer() const;
    bool boolean() const;

private:
    friend class Parser;
    Type type_ = Type::Null;
    bool bool_ = false;
    double number_ = 0.0;
    std::string string_;
    std::vector<Value> array_;
    std::map<std::string, Value> object_;
};

// Parses `text`. On failure returns false and writes "line L, column C: ...".
bool parse(const std::string& text, Value& out, std::string& error);

// Reads and parses a file. Errors name the file plus the parse position.
bool loadFile(const std::filesystem::path& path, Value& out, std::string& error);

} // namespace datafile
