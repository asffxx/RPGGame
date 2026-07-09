#include "economy.hpp"

void Economy::addCurrency(int p, int g, int s, int c) {
    platinum += p;
    gold += g;
    silver += s;
    copper += c;
    upgrade();
}

bool Economy::subtractCurrency(int p, int g, int s, int c) {
    long long totalCopper = (long long)platinum * 100LL * 100 * 100 + (long long)gold * 100 * 100 + (long long)silver * 100 + copper;
    long long needed = (long long)p * 100LL * 100 * 100 + (long long)g * 100 * 100 + (long long)s * 100 + c;
    if (totalCopper < needed) return false;
    totalCopper -= needed;
    platinum = totalCopper / (100LL * 100 * 100);
    totalCopper %= (100LL * 100 * 100);
    gold = totalCopper / (100 * 100);
    totalCopper %= (100 * 100);
    silver = totalCopper / 100;
    copper = totalCopper % 100;
    return true;
}

void Economy::upgrade() {
    while (copper >= 100) {
        copper -= 100;
        silver++;
    }
    while (silver >= 100) {
        silver -= 100;
        gold++;
    }
    while (gold >= 100) {
        gold -= 100;
        platinum++;
    }
}
