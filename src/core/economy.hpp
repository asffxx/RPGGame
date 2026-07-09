#pragma once

// Tracks currency across platinum/gold/silver/copper and normalizes on change.
struct Economy {
    int platinum;
    int gold;
    int silver;
    int copper;

    Economy(int p = 0, int g = 0, int s = 0, int c = 0) : platinum(p), gold(g), silver(s), copper(c) {}

    void addCurrency(int p, int g, int s, int c);
    bool subtractCurrency(int p, int g, int s, int c);

private:
    void upgrade();
};
