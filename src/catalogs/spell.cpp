#include "spell.hpp"
#include "content.hpp"

SpellDatabase::SpellDatabase()
    : spellsList(content::catalog<SpellData>("items/spells.json")) {}

int SpellDatabase::indexOf(const std::string& id) const {
    for (size_t i = 0; i < spellsList.size(); ++i)
        if (spellsList[i].id == id) return static_cast<int>(i);
    return -1;
}

const SpellDatabase::SpellData* SpellDatabase::find(const std::string& id) const {
    int index = indexOf(id);
    return index < 0 ? nullptr : &spellsList[static_cast<size_t>(index)];
}
