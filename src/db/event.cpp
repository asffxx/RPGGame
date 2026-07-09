#include "event.hpp"

eventDatabase::eventDatabase() {
    eventList = {
        { "Find Treasure", "You stumble upon a hidden treasure chest.", 100, 50, 0, 0, false, "", 1},
        { "Ambushed by Bandits", "You are ambushed by a group of bandits!", -50, -20, -10, 0, true, "Cutpurse", 3},
        { "Mysterious Fountain", "You find a mysterious fountain that restores your health and mana.", 0, 0, 30, 20, false, "", 1},
        { "Ambushed by Goblins", "You are ambushed by a group of goblins!", 0, 0, 0, 0, true, "Goblin Marauder", 2},
        { "Cursed Artifact", "You discover a cursed artifact that grants power but drains your life force.", 0, 100, -20, 0, false, "", 1},
        { "Ghostly Encounter", "A restless spirit appears, sharing tales of the past and granting wisdom.", 0, 75, 0, 0, false, "", 1},
        { "Dark Ritual Site", "You find a site of dark rituals. Participating grants forbidden knowledge but at a cost.", 0, 50, 0, -15, false, "", 1},
        { "Undead Ambush", "Skeletal warriors rise from the earth, their bones clattering in the night.", 0, 0, 0, 0, true, "Ancient Remains", 2},
        { "Mysterious Traveler", "A hooded figure offers a cryptic warning and a small reward for your caution.", 25, 25, 0, 0, false, "", 1},
        { "Ancient Ruin", "Exploring an ancient ruin reveals forgotten lore, but triggers a trap.", 0, 40, -10, 0, false, "", 1},
        { "Plague Village", "A village afflicted by a mysterious plague; you catch a glimpse of the horror.", 0, 0, -25, 0, false, "", 1},
        { "Dark Blessing", "A shadowy entity blesses you with dark power, enhancing your abilities at a hidden cost.", 0, 60, -5, 10, false, "", 1},
        { "Merchant Caravan", "You encounter a merchant caravan willing to trade.", 50, 0, 0, 0, false, "", 1},
        { "Healing Spring", "You find a natural spring that heals your wounds.", 0, 0, 50, 0, false, "", 1},
        { "Mana Crystal", "A glowing crystal restores your magical energy.", 0, 0, 0, 50, false, "", 1},
        { "Bandit Camp", "You stumble upon a bandit camp!", 0, 0, 0, 0, true, "Highwayman", 2},
        { "Lost Adventurer", "A lost adventurer shares their story and rewards you.", 30, 30, 0, 0, false, "", 1},
        { "Cursed Forest", "The forest's curse saps your strength.", 0, 0, -15, -10, false, "", 1},
        { "Treasure Map", "You find an old map leading to hidden treasure.", 75, 25, 0, 0, false, "", 1},
        { "Wandering Spirit", "A spirit guides you, granting insight.", 0, 50, 0, 0, false, "", 1},
        { "Ruined Temple", "Exploring a temple drains your mana but reveals secrets.", 0, 30, 0, -20, false, "", 1},
        { "Dragon Sighting", "A dragon appears, ready for battle!", 0, 0, 0, 0, true, "Dragon Hatchling", 1},
        { "Ancient Ruins Trap", "You trigger a trap in ancient ruins.", 0, 0, -20, 0, false, "", 1},
        { "Friendly Merchant", "A merchant offers you a discount.", 50, 25, 0, 0, false, "", 1},
        { "Healing Herbs", "You find healing herbs.", 0, 0, 40, 0, false, "", 1},
        { "Mana Crystal Vein", "You discover a vein of mana crystals.", 0, 0, 0, 60, false, "", 1},
        { "Bandit Ambush", "Bandits ambush you!", 0, 0, 0, 0, true, "Highwayman", 3},
        { "Lost Artifact", "You find a lost artifact granting power.", 0, 80, 0, 0, false, "", 1},
        { "Poisonous Mist", "A poisonous mist surrounds you.", 0, 0, -30, -20, false, "", 1},
        { "Treasure Map Clue", "You find a clue to a treasure map.", 75, 30, 0, 0, false, "", 1},
        { "Wandering Healer", "A healer offers to restore your health.", 0, 0, 100, 0, false, "", 1},
        { "Shadow Creature", "A shadow creature attacks!", 0, 0, 0, 0, true, "Shadow Fiend", 1}
    };
}
