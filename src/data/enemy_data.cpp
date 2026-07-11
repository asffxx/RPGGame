// Enemy template catalog. Included at the end of enemy.cpp so it can populate
// EnemyDatabase::templates without exposing the data through the header.
EnemyDatabase::EnemyDatabase() {
    templates = {
    // FORMAT: NAME, HP, ARMOR, MAGIC ARMOR, ATTACK, MAGIC ATTACK, MANA, MAXMANA, CRITRATE, CRITDAMAGE, EVASIONRATE, MAGICAMPLIFIERRATE, LEVEL, EXP, GOLD, LORE, MINLEVEL, DEBUFFS, DIFFICULTYLEVEL, ALLOWEDLOCATIONS

    { "Ooze",
      {{50, 50, 0.02f, 0.02f, 5, 0, 0, 0, 0.0f, 1.5f, 0.0f, 1.0f, 1, 10, Economy(0,0,0,70)}},
      {"A pulsating ooze, a corrupted mass of slime that dissolves flesh on contact."},
      1, {}, 1, {Terrain, Dungeon} },

    { "Cutpurse",
      {{45, 45, 0.02f, 0.02f, 4, 0, 0, 0, 0.0f, 1.5f, 0.0f, 1.0f, 1, 8, Economy(0,5,0,0)}},
      {"A sly cutpurse, lurking in alleys, stealing not just gold but life essence."},
      1, {}, 1, {PeacefulVillage, PeacefulTown, Terrain} },
    { "Rat Swarm",
      {{40, 40, 0.01f, 0.01f, 4, 0, 0, 0, 0.0f, 1.4f, 0.05f, 1.0f, 1, 6, Economy(0,0,2,40)}},
      {"A mass of diseased rats scurrying as one, gnawing anything in their path."},
      1, {"Poison"}, 1, {Terrain, Dungeon} },

    { "Broken Skeleton",
      {{55, 55, 0.03f, 0.05f, 6, 1, 0, 0, 0.0f, 1.4f, 0.0f, 1.0f, 1, 14, Economy(0,0,8,20)}},
      {"A poorly animated skeleton barely held together by dark magic."},
      1, {}, 1, {Dungeon} },

    { "Wolf",
      {{72, 72, 0.08f, 0.04f, 10, 0, 0, 0, 0.0f, 1.5f, 0.0f, 1.0f, 1, 24, Economy(0,0,10,30)}},
      {"Wolves are fierce predators that hunt in packs, their eyes glowing with feral hunger."},
      1, {}, 2, {Terrain} },

    { "Serpent",
      {{60, 60, 0.03f, 0.03f, 7, 0, 0, 0, 0.0f, 1.5f, 0.0f, 1.0f, 1, 12, Economy(0,0,5,10)}},
      {"A venomous serpent, coiled in shadows, striking with deadly precision."},
      1, {"Poison"}, 2, {Terrain} },

    { "Carrion Bird",
      {{55, 55, 0.02f, 0.02f, 6, 0, 0, 0, 0.0f, 1.5f, 0.0f, 1.0f, 1, 11, Economy(0,0,15,10)}},
      {"A scavenging carrion bird, feasting on the dead, its talons dripping with decay."},
      1, {}, 2, {Terrain} },
    { "Wild Boar",
      {{80, 80, 0.07f, 0.02f, 11, 0, 0, 0, 0.0f, 1.4f, 0.0f, 1.0f, 1, 26, Economy(0,0,15,40)}},
      {"A raging boar that charges without fear, fueled by primal rage."},
      1, {}, 2, {Terrain} },

    { "Goblin Scout",
      {{58, 58, 0.04f, 0.03f, 7, 0, 0, 0, 0.05f, 1.5f, 0.05f, 1.0f, 1, 18, Economy(0,0,20,25)}},
      {"A nimble goblin scout, observing prey before calling reinforcements."},
      1, {}, 2, {PeacefulVillage, Terrain} },


    { "Goblin Marauder",
      {{64, 64, 0.08f, 0.04f, 8, 0, 0, 0, 0.0f, 1.5f, 0.0f, 1.0f, 1, 16, Economy(0,0,30,50)}},
      {"Goblin marauders are savage raiders, pillaging villages under the cover of darkness."},
      1, {}, 3, {PeacefulTown, Terrain, Dungeon} },

    { "Highwayman",
      {{65, 65, 0.05f, 0.02f, 8, 0, 0, 0, 0.0f, 1.5f, 0.0f, 1.0f, 1, 15, Economy(0,1,50,0)}},
      {"A ruthless highwayman, ambushing travelers on cursed roads for their souls."},
      1, {}, 3, {PeacefulTown, Terrain} },

    { "Ancient Remains",
      {{56, 56, 0.04f, 0.08f, 6, 2, 0, 0, 0.0f, 1.5f, 0.0f, 1.0f, 1, 20, Economy(0,0,5,30)}},
      {"Ancient remains are cursed bones of forgotten warriors, animated by dark sorcery."},
      1, {}, 3, {Terrain, Dungeon} },


    { "Wraith",
      {{60, 60, 0.02f, 0.10f, 4, 12, 0, 0, 0.05f, 1.6f, 0.1f, 1.2f, 2, 30, Economy(0,0,20,50)}},
      {"A spectral entity, a tormented soul bound to the mortal realm."},
      2, {"Freeze"}, 4, {Dungeon, Terrain} },

    { "Ghoul",
      {{85, 85, 0.05f, 0.03f, 12, 2, 0, 0, 0.03f, 1.4f, 0.05f, 1.0f, 2, 35, Economy(0,0,25,75)}},
      {"A rotting undead creature, driven by insatiable hunger."},
      2, {"Poison"}, 4, {Dungeon, Terrain} },



    { "Specter",
      {{50, 50, 0.01f, 0.12f, 3, 15, 0, 0, 0.1f, 1.8f, 0.15f, 1.3f, 3, 40, Economy(0,0,30,0)}},
      {"A ghostly apparition, fading in and out of reality."},
      3, {"Magic Dampening"}, 5, {Dungeon, Terrain} },

    { "Vampire",
      {{90, 90, 0.06f, 0.08f, 8, 8, 0, 0, 0.08f, 1.7f, 0.1f, 1.2f, 5, 60, Economy(0,1,50,0)}},
      {"An immortal blood-drinker, elegant yet deadly."},
      5, {"Bleeding"}, 5, {Dungeon} },

    { "Demon",
      {{100, 100, 0.08f, 0.06f, 10, 10, 0, 0, 0.05f, 1.5f, 0.05f, 1.1f, 4, 50, Economy(0,1,0,0)}},
      {"A fiendish being from the abyssal depths."},
      4, {"Burning"}, 6, {Dungeon} },

    { "Lich",
      {{70, 70, 0.03f, 0.15f, 5, 20, 0, 0, 0.05f, 1.6f, 0.05f, 1.4f, 6, 75, Economy(0,2,0,0)}},
      {"A powerful necromancer who cheated death."},
      6, {"Mana Weakness"}, 6, {Dungeon} },

    { "Fire Elemental",
      {{80, 80, 0.05f, 0.05f, 5, 15, 0, 0, 0.05f, 1.5f, 0.05f, 1.2f, 6, 80, Economy(0,0,50,0)}},
      {"A fiery elemental born from volcanic flames, wielding destructive fire magic."},
      6, {"Burning"}, 6, {Terrain} },

    { "Ice Wraith",
      {{60, 60, 0.02f, 0.08f, 4, 10, 0, 0, 0.1f, 1.6f, 0.1f, 1.3f, 3, 45, Economy(0,0,30,0)}},
      {"A spectral being of ice, haunting frozen wastes."},
      3, {"Freeze"}, 3, {Terrain} },

    { "Swamp Beast",
      {{90, 90, 0.08f, 0.04f, 12, 0, 0, 0, 0.03f, 1.4f, 0.02f, 1.0f, 4, 55, Economy(0,0,40,50)}},
      {"A monstrous creature lurking in murky swamps, dripping with venom."},
      4, {"Poison"}, 4, {Terrain} },

    { "Cave Troll",
      {{120, 120, 0.1f, 0.02f, 14, 0, 0, 0, 0.02f, 1.3f, 0.01f, 1.0f, 5, 70, Economy(0,0,60,0)}},
      {"A hulking troll dwelling in dark caves, smashing intruders with brute force."},
      5, {}, 5, {Dungeon, Terrain} },

    { "Necrotic Zombie",
      {{75, 75, 0.06f, 0.06f, 8, 0, 0, 0, 0.0f, 1.2f, 0.0f, 1.0f, 3, 40, Economy(0,0,25,25)}},
      {"A reanimated corpse infused with necrotic energy, spreading decay."},
      3, {"Poison"}, 3, {Dungeon, Terrain} },

    { "Shadow Fiend",
      {{55, 55, 0.01f, 0.12f, 3, 12, 0, 0, 0.15f, 1.8f, 0.2f, 1.4f, 4, 60, Economy(0,0,35,0)}},
      {"A shadowy demon that drains magic and hides in darkness."},
      4, {"Magic Dampening"}, 4, {Dungeon} },

    { "Volcanic Golem",
      {{150, 150, 0.12f, 0.08f, 16, 8, 0, 0, 0.01f, 1.2f, 0.0f, 1.1f, 7, 100, Economy(0,1,0,0)}},
      {"A massive golem forged from volcanic rock, radiating intense heat."},
      7, {"Burning"}, 7, {Dungeon, Terrain} },

    { "Frost Giant",
      {{140, 140, 0.1f, 0.05f, 18, 0, 0, 0, 0.04f, 1.4f, 0.03f, 1.0f, 6, 90, Economy(0,0,80,0)}},
      {"A colossal giant from icy realms, wielding freezing winds."},
      6, {"Freeze"}, 6, {Terrain} },

    { "Plague Bearer",
      {{70, 70, 0.04f, 0.04f, 6, 4, 0, 0, 0.05f, 1.5f, 0.05f, 1.1f, 4, 50, Economy(0,0,45,0)}},
      {"A plague-ridden harbinger, spreading disease and despair."},
      4, {"Poison"}, 4, {Terrain} },

    { "Dragon Hatchling",
      {{100, 100, 0.08f, 0.1f, 10, 12, 0, 0, 0.08f, 1.7f, 0.1f, 1.3f, 8, 120, Economy(0,2,0,0)}},
      {"A young dragon, fierce and powerful, guarding its territory."},
      8, {"Burning"}, 8, {Dungeon} },

    { "Swamp Troll",
      {{140, 140, 0.12f, 0.04f, 16, 2, 0, 0, 0.02f, 1.3f, 0.01f, 1.0f, 6, 95, Economy(0,0,70,0)}},
      {"A hulking troll from the swamps, covered in muck and wielding a massive club."},
      6, {"Poison"}, 6, {Terrain} },

    { "Forest Spirit",
      {{60, 60, 0.02f, 0.08f, 4, 10, 0, 0, 0.1f, 1.6f, 0.15f, 1.2f, 4, 55, Economy(0,0,40,0)}},
      {"A ethereal spirit of the forest, weaving magic and illusions."},
      4, {"Freeze"}, 4, {Terrain} },

    { "Mountain Giant",
      {{160, 160, 0.15f, 0.05f, 20, 0, 0, 0, 0.01f, 1.2f, 0.0f, 1.0f, 7, 110, Economy(0,1,0,0)}},
      {"A colossal giant dwelling in the mountains, hurling boulders at intruders."},
      7, {}, 7, {Terrain} },

    { "Desert Scorpion",
      {{70, 70, 0.04f, 0.04f, 8, 0, 0, 0, 0.05f, 1.5f, 0.05f, 1.1f, 3, 50, Economy(0,0,35,0)}},
      {"A venomous scorpion adapted to the harsh desert, striking with deadly precision."},
      3, {"Poison"}, 3, {Terrain} },

    { "Cave Bat Swarm",
      {{50, 50, 0.01f, 0.01f, 6, 0, 0, 0, 0.2f, 1.8f, 0.2f, 1.0f, 2, 40, Economy(0,0,25,0)}},
      {"A swarm of bats from deep caves, overwhelming foes with numbers."},
      2, {}, 2, {Dungeon} },

    { "Ruins Guardian",
      {{120, 120, 0.1f, 0.12f, 12, 8, 0, 0, 0.04f, 1.4f, 0.03f, 1.1f, 5, 85, Economy(0,0,60,0)}},
      {"An ancient guardian of forgotten ruins, infused with arcane energy."},
      5, {"Magic Dampening"}, 5, {Dungeon} },

    { "Plains Wolf Pack",
      {{90, 90, 0.06f, 0.03f, 12, 0, 0, 0, 0.03f, 1.4f, 0.02f, 1.0f, 4, 65, Economy(0,0,45,0)}},
      {"A pack of wolves roaming the plains, hunting in coordinated attacks."},
      4, {}, 4, {Terrain} },

    { "Volcano Imp",
      {{65, 65, 0.03f, 0.06f, 5, 8, 0, 0, 0.08f, 1.6f, 0.1f, 1.2f, 3, 55, Economy(0,0,40,0)}},
      {"A mischievous imp from volcanic regions, hurling fireballs."},
      3, {"Burning"}, 3, {Terrain} },

    { "Frozen Yeti",
      {{130, 130, 0.1f, 0.08f, 15, 5, 0, 0, 0.03f, 1.3f, 0.02f, 1.0f, 6, 100, Economy(0,0,75,0)}},
      {"A massive yeti from frozen tundras, wielding icy winds."},
      6, {"Freeze"}, 6, {Terrain} },

    { "Giant Spider",
      {{110, 110, 0.03f, 0.03f, 9, 0, 0, 0, 0.0f, 1.5f, 0.1f, 1.0f, 3, 55, Economy(0,0,40,0)}},
      {"A massive spider with venomous fangs, lurking in dark corners."},
      3, {"Poison"}, 3, {Dungeon, Terrain} },

    { "Bandit Leader",
      {{85, 85, 0.06f, 0.04f, 12, 0, 0, 0, 0.05f, 1.6f, 0.05f, 1.0f, 4, 75, Economy(0,1,0,0)}},
      {"A cunning bandit leader, skilled in combat and tactics."},
      4, {}, 4, {PeacefulTown, Terrain} },

    { "Dark Knight",
      {{120, 120, 0.1f, 0.08f, 15, 5, 0, 0, 0.03f, 1.4f, 0.02f, 1.0f, 5, 90, Economy(0,1,50,0)}},
      {"A knight corrupted by dark magic, wielding a cursed blade."},
      5, {"Bleeding"}, 5, {Dungeon} },

    { "Stone Golem",
      {{180, 180, 0.15f, 0.1f, 18, 0, 0, 0, 0.0f, 1.2f, 0.0f, 1.0f, 8, 130, Economy(0,2,0,0)}},
      {"An ancient golem made of stone, impervious to most attacks."},
      8, {}, 8, {Dungeon} },

    { "Fire Imp",
      {{70, 70, 0.02f, 0.04f, 6, 8, 0, 0, 0.08f, 1.7f, 0.15f, 1.2f, 4, 65, Economy(0,0,50,0)}},
      {"A small fiery demon, hurling balls of flame."},
      4, {"Burning"}, 4, {Terrain} },

    { "Necromancer",
      {{80, 80, 0.04f, 0.12f, 4, 18, 0, 0, 0.05f, 1.5f, 0.05f, 1.4f, 7, 110, Economy(0,1,50,0)}},
      {"A master of death magic, raising undead minions."},
      7, {"Mana Weakness"}, 7, {Dungeon} },

    { "Griffin",
      {{140, 140, 0.08f, 0.06f, 16, 0, 0, 0, 0.04f, 1.5f, 0.05f, 1.0f, 6, 105, Economy(0,1,25,0)}},
      {"A majestic beast with eagle wings and lion body, fiercely territorial."},
      6, {}, 6, {Terrain} },

    { "Mermaid",
      {{90, 90, 0.05f, 0.08f, 8, 10, 0, 0, 0.06f, 1.6f, 0.1f, 1.1f, 5, 85, Economy(0,0,75,0)}},
      {"A seductive sea creature, luring sailors to their doom."},
      5, {"Freeze"}, 5, {Terrain} },

    { "Troll Shaman",
      {{160, 160, 0.12f, 0.08f, 14, 12, 0, 0, 0.02f, 1.3f, 0.01f, 1.0f, 7, 125, Economy(0,1,0,0)}},
      {"A shaman troll wielding primal magic and brute strength."},
      7, {"Poison"}, 7, {Terrain, Dungeon} }
    };
}
