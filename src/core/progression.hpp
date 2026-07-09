#pragma once

// Level / experience / skill-point progression for the player.
struct Progression {
    int level;
    float exp;
    int skillPoints;

    Progression(int l = 1, float e = 0.0f, int sp = 0) : level(l), exp(e), skillPoints(sp) {}
};
