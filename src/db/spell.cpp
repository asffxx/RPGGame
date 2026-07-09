#include "spell.hpp"

SpellDatabase::SpellDatabase() {
    spellsList = {
        {
            "Glintstone Pebble", 10,
            22, 0,
            0, 0,
            0.0f, 0.0f,
            0.0f, 0.0f,
            false, {}, 0.0f,
            true, 1.15f,
            1,
            "Fires a shard of glintstone magic at the enemy.",
            0, 0, 50, 0
        },
        {
            "Rock Sling", 14,
            28, 0,
            0, 0,
            0.0f, 0.0f,
            5.0f, 0.0f,
            true, {"Armor Break"}, 1.0f,
            false, 1.0f,
            2,
            "Hurls boulders that stagger and weaken armor.",
            0, 0, 50, 0
        },
        {
            "Flame Sling", 12,
            20, 0,
            0, 0,
            0.0f, 0.0f,
            0.0f, 0.0f,
            true, {"Burning"}, 1.0f,
            false, 1.0f,
            1,
            "Launches fire that scorches enemies over time.",
            0, 0, 50, 0
        },
        {
            "Lightning Spear", 18,
            32, 0,
            0, 0,
            0.0f, 0.0f,
            0.0f, 5.0f,
            true, {"Shock"}, 1.0f,
            false, 1.0f,
            3,
            "Conjures a spear of lightning that pierces foes.",
            0, 0, 50, 0
        },
        {
            "Urgent Heal", 14,
            0, 0,
            40, 0,
            0.0f, 0.0f,
            0.0f, 0.0f,
            false, {}, 0.0f,
            false, 1.0f,
            1,
            "Quickly restores health in moments of danger.",
            0, 0, 50, 0
        },
        {
            "Flame, Grant Me Strength", 22,
            0, 0,
            0, 0,
            8.0f, 4.0f,
            0.0f, 0.0f,
            false, {}, 0.0f,
            false, 1.0f,
            4,
            "Bolsters the body with flame, increasing defenses.",
            0, 0, 50, 0
        },
        {
            "Comet Azur", 30,
            45, 0,
            0, 0,
            0.0f, 0.0f,
            0.0f, 8.0f,
            true, {"Freeze"}, 1.2f,
            true, 1.25f,
            6,
            "Unleashes a devastating torrent of magic.",
            0, 0, 50, 0
        },
        {
            "Stars of Ruin", 38,
            50, 0,
            0, 0,
            0.0f, 0.0f,
            0.0f, 0.0f,
            true, {"Burning"}, 1.0f,
            true, 1.2f,
            7,
            "Summons homing stars that relentlessly pursue foes.",
            0, 0, 50, 0
        },
        {
            "Fireball", 20,
            38, 0,
            0, 0,
            0.0f, 0.0f,
            0.0f, 0.0f,
            true, {"Burning"}, 1.0f,
            false, 1.0f,
            2,
            "Hurls a blazing fireball that explodes on impact.",
            0, 0, 50, 0
        },
        {
            "Greater Heal", 28,
            0, 0,
            85, 0,
            0.0f, 0.0f,
            0.0f, 0.0f,
            false, {}, 0.0f,
            false, 1.0f,
            5,
            "A powerful incantation that restores great vitality.",
            0, 0, 50, 0
        },
        {
            "Frost Nova", 24,
            18, 0,
            0, 0,
            0.0f, 0.0f,
            0.0f, 0.0f,
            true, {"Freeze"}, 1.0f,
            false, 1.0f,
            4,
            "Releases a wave of frost that freezes nearby enemies.",
            0, 0, 50, 0
        }
    };
}
