#include "potion.hpp"
#include "content.hpp"

PotionDatabase::PotionDatabase()
    : potionList(content::catalog<potionProperties>("items/potions.json")) {}

int PotionDatabase::indexOf(const std::string& id) const {
    for (size_t i = 0; i < potionList.size(); ++i)
        if (potionList[i].id == id) return static_cast<int>(i);
    return -1;
}

const PotionDatabase::potionProperties* PotionDatabase::find(const std::string& id) const {
    int index = indexOf(id);
    return index < 0 ? nullptr : &potionList[static_cast<size_t>(index)];
}
