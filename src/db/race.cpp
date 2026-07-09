#include "race.hpp"

PlayerRaceDatabase::PlayerRaceDatabase() {
    templates = {

        { "Human", {5, 5, 0.025f, 0.025f, 1, 1, 2, 2, 0, 0, 0, 0, 1, 0.0f},
            {"Humans are versatile and resilient, gaining a slight boost to all stats."} },
        { "Noble", {4, 4, 0.025f, 0.025f, 1, 1, 2, 2, 0, 0, 0, 0, 1, 0.0f},
            {"Nobles are trained in etiquette and combat, slightly higher attack and mana."} },
        { "Commoner", {6, 6, 0.02f, 0.02f, 0, 0, 2, 2, 0, 0, 0, 0, 1, 0.0f},
            {"Commoners are hardy but less trained in combat, slightly higher hitpoints."} },


        { "Elf", {0, 0, 0.0f, 0.05f, 2, 5, 0, 0, 0, 0, 0, 0, 1, 0.0f},
            {"Elves are agile and magically attuned, gaining bonus magic attack and magic armor."} },
        { "High Elf", {0, 0, 0.0f, 0.06f, 3, 6, 0, 0, 0, 0, 0, 0, 1, 0.0f},
            {"High Elves are elite magic users, with enhanced magic abilities."} },
        { "Wood Elf", {0, 0, 0.0f, 0.04f, 2, 4, 0, 0, 0, 0, 0, 0, 1, 0.0f},
            {"Wood Elves are skilled hunters and scouts, with increased agility and stealth."} },

        { "Dark Elf", {2, 2, 0.0f, 0.075f, 3, 6, 0, 0, 0, 0, 0, 0, 1, 0.0f},
            {"Dark Elves are exiled elves who dwell in shadows and forbidden lands, mistrusted by other races."} },


        { "Orc", {15, 15, 0.075f, 0.0f, 5, 0, 0, 0, 0, 0, 0, 0, 1, 0.0f},
            {"Orcs are strong, aggressive warriors often hunted or outlawed by human and elf societies."} },
        { "Reptilian", {15, 15, 0.075f, 0.0f, 5, 0, 0, 0, 0, 0, 0, 0, 1, 0.0f},
            {"Reptilians are predatory creatures, often hunted by human and elf societies."} }
     };
}
