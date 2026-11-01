#include "class.hpp"
#include "content.hpp"

PlayerClassCollection::PlayerClassCollection()
    : templates(content::catalog<PlayerClassTemplate>("characters/classes.json")) {}

const PlayerClassTemplate* PlayerClassCollection::findById(const std::string& id) const {
    for (const auto& cls : templates)
        if (cls.id == id) return &cls;
    return nullptr;
}

const PlayerClassTemplate& PlayerClassCollection::getClassTemplate(const std::string& className) const {
    for (const auto& cls : templates) {
        if (cls.name == className) return cls;
    }
    throw std::runtime_error("Class template not found: " + className);
}
