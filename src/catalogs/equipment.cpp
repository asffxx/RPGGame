#include "equipment.hpp"
#include "content.hpp"

EquipmentandWeaponDatabase::EquipmentandWeaponDatabase()
    : equipmentList(content::catalog<equipmentProperties>("items/equipment.json")) {}

int EquipmentandWeaponDatabase::indexOf(const std::string& id) const {
    for (size_t i = 0; i < equipmentList.size(); ++i)
        if (equipmentList[i].id == id) return static_cast<int>(i);
    return -1;
}

const EquipmentandWeaponDatabase::equipmentProperties*
EquipmentandWeaponDatabase::find(const std::string& id) const {
    int index = indexOf(id);
    return index < 0 ? nullptr : &equipmentList[static_cast<size_t>(index)];
}
