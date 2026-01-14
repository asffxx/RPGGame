#include <iostream>
#include <string>
#include <limits>
#include <random>
#include <vector>
#include <cmath>
#include <algorithm>
#include <optional>
#include <chrono>
#include <cctype>
#include <thread>
#include <future>
#include <set>
#include <map>
#include <functional>
#include <cstdlib>

using std::cout;
using std::cin;
using std::string;
using std::endl;

enum LocationType { PeacefulVillage, PeacefulTown, Dungeon, Terrain, SpellStore };
class EquipmentandWeaponDatabase;
class NPCGenerator;
class Player;
class TimeSystem;
struct NPC;
void deductWeeklyWages(Player& player, std::vector<NPC>& playerParty);
void narrate();

void displayBorderedMenu(const std::vector<string>& lines, const string& prompt = "Choose an option: ") {
    size_t maxWidth = 0;
    for (const auto& line : lines) {
        maxWidth = std::max(maxWidth, line.length());
    }
    string border = string(maxWidth, '=');
    cout << border << endl;
    for (const auto& line : lines) {
        cout << line << string(maxWidth - line.length(), ' ') << endl;
    }
    cout << border << endl;
    cout << prompt;
}

class PagedSelector {
public:
   
    PagedSelector(const std::vector<string>& itemList, size_t pageSizeParam = 5)
        : items(itemList), pageSize(pageSizeParam), pageStart(0) {}

    size_t select() {
        size_t selectedIndex = 0;
        bool choosing = true;

        while (choosing) {
            size_t pageEnd = std::min<size_t>(pageStart + pageSize, items.size());

            for (size_t i = pageStart; i < pageEnd; ++i) {
                cout << i + 1 << ". " << items[i] << endl;
            }

            cout << "Enter number to select, or input a number out of range to scroll (negative for prev, >" << items.size() << " for next):" << endl;
            string input;
            getline(cin, input);

            int choice = 0;
            try {
                choice = std::stoi(input);
            } catch (...) {
                cout << "Invalid input! Enter a number.\n";
                continue;
            }

            if (choice < 1) {
               
                if (pageStart > 0) {
                    pageStart = std::max<size_t>(0, pageStart - pageSize);
                } else {
                    cout << "Already on first page.\n";
                }
            } else if (choice > static_cast<int>(items.size())) {
               
                if (pageStart + pageSize < items.size()) {
                    pageStart += pageSize;
                } else {
                    cout << "Already on last page.\n";
                }
            } else if (choice > static_cast<int>(pageEnd) || choice <= static_cast<int>(pageStart)) {
               
                pageStart = (static_cast<size_t>(choice) - 1) / pageSize * pageSize;
            } else {
                selectedIndex = static_cast<size_t>(choice) - 1;
                choosing = false;
            }
        }

        return selectedIndex;
    }

private:
    std::vector<string> items;
    size_t pageSize;
    size_t pageStart;
};

int getNumberInput(int min, int max) {
    while (true) {
        string input;
        getline(std::cin, input);

        try {
            int choice = std::stoi(input); 
            if (choice >= min && choice <= max)
                return choice;
            else
                cout << "Please enter a number between " << min << " and " << max << ": ";
        } catch (...) {
            cout << "Invalid input! Enter a number: ";
        }
    }
}

struct Stats {
    int hitpoints;
    int maxHitpoints;
    float armor;
    float magicArmor;
    int attack;
    int magicAttack;
    int mana;
    int maxMana;
    float critRate;
    float critDamage;
    float dodgeRate;
    float magicAmplifierRate;
    int level;
    float expe;

    void scale(float multiplier) {
        maxHitpoints = static_cast<int>(std::round(maxHitpoints * multiplier));
        maxMana = static_cast<int>(std::round(maxMana * multiplier));
        hitpoints = static_cast<int>(std::round(hitpoints * multiplier));
        hitpoints = std::min<int>(hitpoints, maxHitpoints);
        mana = static_cast<int>(std::round(mana * multiplier));
        mana = std::min<int>(mana, maxMana);
        armor *= multiplier;
        magicArmor *= multiplier;
        attack = static_cast<int>(std::round(attack * multiplier));
        magicAttack = static_cast<int>(std::round(magicAttack * multiplier));
        dodgeRate *= multiplier;
        critRate *= multiplier;
        critDamage *= multiplier;
        magicAmplifierRate *= multiplier;
    }
};

struct Economy {
    int platinum;
    int gold;
    int silver;
    int copper;

    Economy(int p = 0, int g = 0, int s = 0, int c = 0) : platinum(p), gold(g), silver(s), copper(c) {}

    void addCurrency(int p, int g, int s, int c) {
        platinum += p;
        gold += g;
        silver += s;
        copper += c;
        upgrade();
    }

    bool subtractCurrency(int p, int g, int s, int c) {
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

private:
    void upgrade() {
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
};

struct Progression {
    int level;
    float exp;
    int skillPoints;

    Progression(int l = 1, float e = 0.0f, int sp = 0) : level(l), exp(e), skillPoints(sp) {}
};

struct Lore {
    string description;
    Lore(const string& desc = "") : description(desc) {}
};

struct PlayerRace {
    string name;
    Lore lore;
};
                           
class TimeSystem {
public:
    enum class TimePeriod { Morning, Afternoon, Evening, Night };

    TimeSystem() : currentTurn(0), currentPeriod(TimePeriod::Morning), totalDays(0), totalWeeks(0) {}

    void advanceTime(Player& player);

    string getPeriodString() const {
        switch (currentPeriod) {
            case TimePeriod::Morning: return "Morning";
            case TimePeriod::Afternoon: return "Afternoon";
            case TimePeriod::Evening: return "Evening";
            case TimePeriod::Night: return "Night";
            default: return "Unknown";
        }
    }

    string getFormattedTimePassed() const {
        int daysPassed = getTotalDays();
        if (daysPassed == 0) return "0 days";

        string result;
        int years = daysPassed / (12 * 4 * 7);
        int remaining = daysPassed % (12 * 4 * 7);
        int months = remaining / (4 * 7);
        remaining %= (4 * 7);
        int weeks = remaining / 7;
        int days = remaining % 7;

        if (years > 0) result += std::to_string(years) + " years ";
        if (months > 0) result += std::to_string(months) + " months ";
        if (weeks > 0) result += std::to_string(weeks) + " weeks ";
        if (days > 0) result += std::to_string(days) + " days ";

        if (!result.empty()) result.pop_back(); 
        return result;
    }

    TimePeriod getCurrentPeriod() const {
        return currentPeriod;
    }

    int getCurrentTurn() const {
        return currentTurn;
    }

    int getTotalDays() const {
        return totalDays;
    }

    int getTotalWeeks() const {
        return totalWeeks;
    }

    void setState(int turn, TimePeriod period, int days) {
        currentTurn = turn;
        currentPeriod = period;
        totalDays = days;
        totalWeeks = totalDays / 7;
    }

private:
    int currentTurn;
    TimePeriod currentPeriod;
    int totalDays;
    int totalWeeks;
};

struct RaceLore {
    string description;
};

struct PlayerRaceTemplate {
    string name;
    Stats statBonus; 
    RaceLore lore; 
};

struct PlayerClassTemplate {
    string name;
    Stats stats;
    Lore lore;
};

class Player {
public:
    string name;
    Stats stats;
    Economy economy;
    Progression progression;
    PlayerRace race;
    PlayerClassTemplate playerClass;
    TimeSystem timeSystem;
    std::vector<string> debuffs;
    std::vector<string> equippedWeaponDebuffs;
    float equippedWeaponDebuffChance;
    LocationType currentLocationType;
    string currentLocation;
    std::vector<string> learnedSpells;
    std::vector<string> equippedStaffSpells;
    bool sleptToday;
    std::set<string> defeatedEnemies;
    std::set<string> discoveredLocations;
    std::set<string> boughtWeapons;
    std::set<string> hiredSpecialCharacters;
    std::set<string> encounteredEvents;
    bool hasNewDictionaryEntry = false;

    Player(const string& n,const PlayerRace& r,const PlayerClassTemplate& pc): name(n), race(r), playerClass(pc), timeSystem(), currentLocationType(Terrain), currentLocation("Abandoned Cell") {
        stats = pc.stats; 
        economy = Economy();
        progression = Progression();
    }
    void applyRaceBonus(const Stats& bonus) {
    stats.hitpoints += bonus.hitpoints;
    stats.maxHitpoints += bonus.maxHitpoints;
    stats.armor += bonus.armor;
    stats.magicArmor += bonus.magicArmor;
    stats.attack += bonus.attack;
    stats.magicAttack += bonus.magicAttack;
    stats.mana += bonus.mana;
    stats.maxMana += bonus.maxMana;
}

    void scaleStats(float multiplier) {
        stats.scale(multiplier);
    }
};

void TimeSystem::advanceTime(Player& player) {
    currentTurn++;
    switch (currentPeriod) {
        case TimePeriod::Morning:
            currentPeriod = TimePeriod::Afternoon;
            break;
        case TimePeriod::Afternoon:
            currentPeriod = TimePeriod::Evening;
            break;
        case TimePeriod::Evening:
            currentPeriod = TimePeriod::Night;
            break;
        case TimePeriod::Night:
            currentPeriod = TimePeriod::Morning;
            totalDays++;
            totalWeeks = totalDays / 7;
        
            if (!player.sleptToday) {
                int penalty = static_cast<int>(std::round(player.stats.maxHitpoints * 0.05f));
                player.stats.hitpoints -= penalty;
                if (player.stats.hitpoints < 1) player.stats.hitpoints = 1;
                cout << "[!] Your vision blurs. The shadows of 'The Lands Between' seem longer than usual." << endl;
            }

            player.sleptToday = false;
            break;
    }
}

class PlayerClassCollection {
public:
    std::vector<PlayerClassTemplate> templates;

    PlayerClassCollection() {
        templates = {
            {"Warrior", {75, 75, 0.05f, 0.02f, 8, 2, 10, 10, 0.05f, 1.5f, 0.05f, 1.0f, 1, 0.0f},
             {"Strong and resilient melee fighter, excels at absorbing damage."}},
            {"Knight", {70, 70, 0.08f, 0.03f, 10, 2, 8, 8, 0.04f, 1.4f, 0.05f, 1.0f, 1, 0.0f},
             {"Heavily armored, balanced between defense and offense."}},
            {"Berserker", {80, 80, 0.04f, 0.02f, 7, 2, 6, 6, 0.03f, 1.3f, 0.05f, 1.0f, 1, 0.0f},
             {"Frenzied melee attacker with high hitpoints and raw power."}},

            {"Assassin", {55, 55, 0.02f, 0.01f, 8, 2, 6, 6, 1.0f, 100.0f, 1.0f, 1.0f, 1, 0.0f},
             {"Agile and deadly, specializes in critical strikes."}},
            {"Rogue", {60, 60, 0.02f, 0.01f, 7, 2, 8, 8, 0.2f, 1.8f, 0.1f, 1.0f, 1, 0.0f},
             {"Stealthy fighter with high dexterity and precision."}},
            {"Ranger", {50, 50, 0.01f, 0.01f, 5, 4, 10, 10, 0.3f, 1.7f, 0.1f, 1.0f, 1, 0.0f},
             {"Skilled marksman, excels at ranged attacks and high crits."}},

            {"Mage", {45, 45, 0.01f, 0.05f, 2, 9, 18, 18, 0.05f, 1.5f, 0.05f, 1.2f, 1, 0.0f},
             {"Master of arcane arts, high magic attack and mana pool."}},
            {"Sorcerer", {40, 40, 0.01f, 0.07f, 2, 11, 22, 22, 0.05f, 1.5f, 0.05f, 1.3f, 1, 0.0f},
             {"Specialist in destructive spells, fragile but powerful."}},
            {"Scholar", {50, 50, 0.02f, 0.05f, 3, 7, 14, 14, 0.05f, 1.5f, 0.05f, 1.1f, 1, 0.0f},
             {"Versatile magic user, balanced between offense and defense."}},

            {"Shrewd Diplomat", {60, 60, 0.02f, 0.03f, 5, 4, 14, 14, 0.05f, 1.5f, 0.05f, 1.0f, 1, 0.0f},
             {"Uses wit and charm to outsmart opponents, prefers diplomacy over fighting."}},
            {"Travelling Merchant", {65, 65, 0.03f, 0.02f, 3, 2, 10, 10, 0.05f, 1.5f, 0.05f, 1.0f, 1, 0.0f},
             {"Journeys across lands trading goods, can defend himself when needed."}},
            {"Sheltered Noble", {55, 55, 0.02f, 0.05f, 3, 2, 10, 10, 0.05f, 1.5f, 0.05f, 1.0f, 1, 0.0f},
             {"Raised in comfort, trained in etiquette and light combat."}},
            {"Prisoner", {50, 50, 0.01f, 0.01f, 2, 2, 6, 6, 0.05f, 1.2f, 0.05f, 1.0f, 1, 0.0f},
             {"Imprisoned for unknown reasons, weak but resilient in spirit."}}
        };
    }

    const PlayerClassTemplate& getClassTemplate(const string& className) const {
        for (const auto& cls : templates) {
            if (cls.name == className) return cls;
        }
        throw std::runtime_error("Class template not found: " + className);
    }
};

struct PlayerRaceDatabase {
    std::vector<PlayerRaceTemplate> templates;

    PlayerRaceDatabase() {
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
};

struct NPC {
    string name;
    Stats stats;
    PlayerRace race;
    PlayerClassTemplate playerClass;
    int level;
    std::vector<string> inventory;
    string equippedWeapon;
    string equippedArmor;
    std::vector<string> debuffs;
    std::vector<string> spells;
    int wagePerWeek;
    string story;
    std::vector<string> dialogues;
};

class FoodandDrinksDatabase{
    public:
    struct FoodsandDrinksProperties{
            string name;
            string type;
            int healthRestoration;
            int maxHealthBuffBonus;
            int maxManaBuffBonus;
            int manaRestoration;
            string itemDesc;

            int pricePlatinum;
            int priceGold;
            int priceSilver;
            int priceCopper;
        };

        const std::vector<FoodsandDrinksProperties>& getFoodAndDrink() const {
            return FoodandDrinkMenu;
        }
    private:
    std::vector<FoodsandDrinksProperties> FoodandDrinkMenu{
        // Foods
        {"Bread", "Food", 20, 0, 0, 0, "A loaf of bread that restores some health.", 0,0,1,0},
        {"Apple", "Food", 25, 0, 0, 0, "A fresh apple that restores health.", 0,0,1,50},
        {"Cheese", "Food", 30, 0, 0, 0, "A piece of cheese that restores health.", 0,0,2,0},
        // Dishes
        {"Meat Stew", "Dish", 50, 5, 0, 0, "A hearty stew that restores health and boosts max HP.", 0,0,5,0},
        {"Grilled Fish", "Dish", 45, 3, 0, 0, "Grilled fish that restores health.", 0,0,6,0},
        {"Vegetable Soup", "Dish", 40, 2, 0, 0, "Nutritious soup that restores health.", 0,0,4,50},
        {"Roasted Meat", "Dish", 60, 7, 0, 0, "Roasted meat that greatly restores health.", 0,0,8,0},
        {"Salad", "Dish", 35, 1, 0, 0, "A fresh salad that restores health.", 0,0,3,50},
        // Drinks
        {"Water", "Drink", 0, 0, 0, 10, "Clean water that restores mana.", 0,0,0,50},
        {"Milk", "Drink", 0, 0, 0, 15, "Fresh milk that restores mana.", 0,0,1,0},
        {"Wine", "Drink", 0, 0, 0, 20, "A glass of wine that restores mana.", 0,0,3,0},
        {"Beer", "Drink", 0, 0, 0, 18, "A mug of beer that restores mana.", 0,0,2,50},
        {"Juice", "Drink", 0, 0, 0, 12, "Fruit juice that restores mana.", 0,0,1,50}
    };
};

class PotionDatabase {
    public:
        struct potionProperties{
            string name;
            int hpEffect = 0;
            int attackEffect = 0;
            int defenseEffect = 0;
            int magicEffect = 0;
            int manaRestore = 0;
            int manaIncreaseMax = 0;
            int magicDefenseEffect = 0;
            float critRateEffect = 0.0f;
            float critDamageEffect = 0.0f;
            int durationTurns = 0;
            string effectDesc;
            int pricePlatinum;
            int priceGold;
            int priceSilver;
            int priceCopper;
        };

        const std::vector<potionProperties>& getPotions() const {
            return potionList;
        }

    private:
        std::vector<potionProperties> potionList{
        { "Small Health Potion", 50, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0, "Restores 50 HP", 0, 0, 12, 0 },
        { "Large Health Potion", 150, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0, "Restores 150 HP", 0, 0, 36, 0 },
        { "Extra Large Health Potion", 300, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0, "Restores 300 HP", 0, 0, 72, 0 },
        { "Small Mana Potion", 0, 0, 0, 0, 15, 5, 0, 0.0f, 0.0f, 3, "Restores 15 Mana for 3 turns", 0, 0, 24, 0 },
        { "Large Mana Potion", 0, 0, 0, 0, 40, 10, 0, 0.0f, 0.0f, 3, "Restores 40 Mana for 3 turns", 0, 0, 60, 0 },
        { "Extra Large Mana Potion", 0, 0, 0, 0, 80, 20, 0, 0.0f, 0.0f, 3, "Restores 80 Mana for 3 turns", 0, 0, 120, 0 },
        { "Strength Potion", 0, 10, 0, 0, 0, 0, 0, 0.0f, 0.0f, 3, "Increases Attack by 10 for 3 turns", 0, 0, 30, 0 },
        { "Defense Potion", 0, 0, 5, 0, 0, 0, 0, 0.0f, 0.0f, 3, "Increases Defense by 5 for 3 turns", 0, 0, 30, 0 },
        { "Magic Elixir", 0, 0, 0, 15, 20, 10, 0, 0.0f, 0.0f, 3, "Increases Magic Attack by 15 and restores 20 Mana for 3 turns", 0, 0, 48, 0 },
        { "Rage Potion", 0, 0, 0, 0, 0, 0, 0, 0.1f, 0.5f, 3, "Increases Crit Rate by 10% and Crit Damage by 50% for 3 turns", 0, 0, 96, 0 },
        { "Wrath Potion", 0, 0, 0, 0, 0, 0, 0, 0.15f, 0.75f, 3, "Increases Crit Rate by 15% and Crit Damage by 75% for 3 turns", 0, 0, 108, 0 },
        { "Speed Potion", 0, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 3, "Increases Dodge Rate by 20% for 3 turns", 0, 0, 42, 0 },
        { "Regeneration Potion", 20, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 5, "Restores 20 HP per turn for 5 turns", 0, 0, 84, 0 },
        { "Invisibility Potion", 0, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 2, "Increases Dodge Rate by 50% for 2 turns", 0, 0, 150, 0 }
    };
};

class EquipmentandWeaponDatabase {
    public:
        struct equipmentProperties{
            string name;
            string type;
            int defenseIncrease;
            int magicDefenseIncrease;
            int attackIncrease;
            int magicAttackIncrease;
            float critRateIncrease;
            float critDamageIncrease;
            std::vector<string> debuffs;
            float debuffChance;
            string effectDesc;
            int pricePlatinum;
            int priceGold;
            int priceSilver;
            int priceCopper;
        };

        const std::vector<equipmentProperties>& getEquipment() const {
            return equipmentList;
        }

    private:
    std::vector<equipmentProperties> equipmentList{
        { "Leather Armor", "Armor", 5, 0, 0, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Defense by 5", 0, 0, 24, 0 },
        { "Reinforced Leather Armor", "Armor", 7, 0, 0, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Defense by 7", 0, 0, 36, 0 },
        { "Iron Armor", "Armor", 10, 0, 0, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Defense by 10", 0, 0, 60, 0 },
        { "Chainmail", "Armor", 12, 0, 0, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Defense by 12", 0, 0, 72, 0 },
        { "Steel Armor", "Armor", 15, 0, 0, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Defense by 15", 0, 0, 108, 0 },
        { "Steel Sword", "Weapon", 0, 0, 22, 0, 0.0f, 0.0f, {"Burning"}, 0.3f, "Increases Attack by 22, 30% chance to apply Burning debuff.", 0, 0, 132, 0 },
        { "Wooden Sword", "Weapon", 0, 0, 8, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 8", 0, 0, 36, 0 },
        { "Iron Sword", "Weapon", 0, 0, 15, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 15", 0, 0, 90, 0 },
        { "Dagger", "Weapon", 0, 0, 5, 0, 0.1f, 1.8f, {}, 0.0f, "Increases Attack by 5, Crit Rate by 10%, Crit Damage by 80%", 0, 0, 72, 0 },
        { "Enchanted Dagger", "Weapon", 0, 0, 8, 0, 0.15f, 2.0f, {}, 0.0f, "Increases Attack by 8, Crit Rate by 15%, Crit Damage by 100%", 0, 0, 126, 0 },
        { "Magic Staff", "Staff", 0, 22, 0, 22, 0.0f, 0.0f, {}, 0.0f, "Increases Magic Attack by 22", 0, 1, 32, 0 },
        { "Wooden Staff", "Staff", 0, 0, 0, 8, 0.0f, 0.0f, {}, 0.0f, "Increases Magic Attack by 8", 0, 0, 36, 0 },
        { "Iron Staff", "Staff", 0, 0, 0, 15, 0.0f, 0.0f, {}, 0.0f, "Increases Magic Attack by 15", 0, 0, 90, 0 },
        { "Greatsword", "Weapon", 0, 0, 20, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 20", 0, 1, 20, 0 },
        { "Mace", "Weapon", 0, 0, 12, 0, 0.0f, 0.0f, {"Weakness"}, 0.2f, "Increases Attack by 12, 20% chance to apply Weakness debuff.", 0, 0, 72, 0 },
        { "Battle Axe", "Weapon", 0, 0, 16, 0, 0.0f, 0.0f, {"Bleeding"}, 0.25f, "Increases Attack by 16, 25% chance to apply Bleeding debuff.", 0, 0, 96, 0 },
        { "Longsword", "Weapon", 0, 0, 18, 0, 0.05f, 0.0f, {}, 0.0f, "Increases Attack by 18, slight Crit Rate bonus.", 0, 0, 108, 0 },
        { "Rapier", "Weapon", 0, 0, 12, 0, 0.15f, 2.0f, {}, 0.0f, "Increases Attack by 12, high Crit Rate and Damage.", 0, 0, 84, 0 },
        { "War Hammer", "Weapon", 0, 0, 20, 0, 0.0f, 0.0f, {"Weakness"}, 0.3f, "Increases Attack by 20, 30% chance to apply Weakness debuff.", 0, 0, 120, 0 },
        { "Halberd", "Weapon", 0, 0, 24, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 24, powerful polearm.", 0, 1, 0, 0 },
        { "Crossbow", "Weapon", 0, 0, 14, 0, 0.1f, 1.5f, {}, 0.0f, "Increases Attack by 14, ranged weapon with Crit bonuses.", 0, 0, 90, 0 },
        { "Morning Star", "Weapon", 0, 0, 17, 0, 0.0f, 0.0f, {"Bleeding"}, 0.35f, "Increases Attack by 17, 35% chance to apply Bleeding debuff.", 0, 0, 102, 0 },
        { "Katana", "Weapon", 0, 0, 19, 0, 0.08f, 1.8f, {}, 0.0f, "Increases Attack by 19, balanced Crit stats.", 0, 0, 114, 0 },
        { "Flail", "Weapon", 0, 0, 15, 0, 0.0f, 0.0f, {"Weakness"}, 0.4f, "Increases Attack by 15, 40% chance to apply Weakness debuff.", 0, 0, 90, 0 },
        { "Spear", "Weapon", 0, 0, 13, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 13, versatile polearm.", 0, 0, 78, 0 },
        { "Bow", "Weapon", 0, 0, 11, 0, 0.12f, 1.6f, {}, 0.0f, "Increases Attack by 11, ranged with good Crit stats.", 0, 0, 66, 0 },
        { "Scimitar", "Weapon", 0, 0, 14, 0, 0.06f, 1.4f, {}, 0.0f, "Increases Attack by 14, curved blade with Crit bonuses.", 0, 0, 84, 0 },
        { "Claymore", "Weapon", 0, 0, 22, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 22, heavy two-handed sword.", 0, 1, 20, 0 },
        { "Falchion", "Weapon", 0, 0, 16, 0, 0.04f, 1.2f, {}, 0.0f, "Increases Attack by 16, slashing weapon with Crit.", 0, 0, 96, 0 },
        { "Pike", "Weapon", 0, 0, 18, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 18, long reach polearm.", 0, 0, 108, 0 },
        { "Throwing Knives", "Weapon", 0, 0, 8, 0, 0.2f, 2.5f, {"Bleeding"}, 0.15f, "Increases Attack by 8, high Crit, 15% chance Bleeding.", 0, 0, 48, 0 },
        { "Mace", "Weapon", 0, 0, 12, 0, 0.0f, 0.0f, {"Weakness"}, 0.2f, "Increases Attack by 12, 20% chance to apply Weakness debuff.", 0, 0, 72, 0 },
        { "Battle Axe", "Weapon", 0, 0, 16, 0, 0.0f, 0.0f, {"Bleeding"}, 0.25f, "Increases Attack by 16, 25% chance to apply Bleeding debuff.", 0, 0, 96, 0 },
        { "Longsword", "Weapon", 0, 0, 18, 0, 0.05f, 0.0f, {}, 0.0f, "Increases Attack by 18, slight Crit Rate bonus.", 0, 0, 108, 0 },
        { "Rapier", "Weapon", 0, 0, 12, 0, 0.15f, 2.0f, {}, 0.0f, "Increases Attack by 12, high Crit Rate and Damage.", 0, 0, 84, 0 },
        { "War Hammer", "Weapon", 0, 0, 20, 0, 0.0f, 0.0f, {"Weakness"}, 0.3f, "Increases Attack by 20, 30% chance to apply Weakness debuff.", 0, 0, 120, 0 },
        { "Halberd", "Weapon", 0, 0, 24, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 24, powerful polearm.", 0, 1, 0, 0 },
        { "Crossbow", "Weapon", 0, 0, 14, 0, 0.1f, 1.5f, {}, 0.0f, "Increases Attack by 14, ranged weapon with Crit bonuses.", 0, 0, 90, 0 },
        { "Morning Star", "Weapon", 0, 0, 17, 0, 0.0f, 0.0f, {"Bleeding"}, 0.35f, "Increases Attack by 17, 35% chance to apply Bleeding debuff.", 0, 0, 102, 0 },
        { "Katana", "Weapon", 0, 0, 19, 0, 0.08f, 1.8f, {}, 0.0f, "Increases Attack by 19, balanced Crit stats.", 0, 0, 114, 0 },
        { "Flail", "Weapon", 0, 0, 15, 0, 0.0f, 0.0f, {"Weakness"}, 0.4f, "Increases Attack by 15, 40% chance to apply Weakness debuff.", 0, 0, 90, 0 },
        { "Spear", "Weapon", 0, 0, 13, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 13, versatile polearm.", 0, 0, 78, 0 },
        { "Bow", "Weapon", 0, 0, 11, 0, 0.12f, 1.6f, {}, 0.0f, "Increases Attack by 11, ranged with good Crit stats.", 0, 0, 66, 0 },
        { "Scimitar", "Weapon", 0, 0, 14, 0, 0.06f, 1.4f, {}, 0.0f, "Increases Attack by 14, curved blade with Crit bonuses.", 0, 0, 84, 0 },
        { "Claymore", "Weapon", 0, 0, 22, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 22, heavy two-handed sword.", 0, 1, 20, 0 },
        { "Falchion", "Weapon", 0, 0, 16, 0, 0.04f, 1.2f, {}, 0.0f, "Increases Attack by 16, slashing weapon with Crit.", 0, 0, 96, 0 },
        { "Pike", "Weapon", 0, 0, 18, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 18, long reach polearm.", 0, 0, 108, 0 },
        { "Throwing Knives", "Weapon", 0, 0, 8, 0, 0.2f, 2.5f, {"Bleeding"}, 0.15f, "Increases Attack by 8, high Crit, 15% chance Bleeding.", 0, 0, 48, 0 },
        { "Quarterstaff", "Weapon", 0, 0, 10, 0, 0.0f, 0.0f, {}, 0.0f, "Increases Attack by 10, simple but effective.", 0, 0, 60, 0 }
    }; };

namespace debuffSettings{
    struct DebuffEffect {
        int healthDecrease = 0;
        int attackDecrease = 0;
        int defenseDecrease = 0;
        int magicAttackDecrease = 0;
        int magicDefenseDecrease = 0;

        int durationTurns = 0;
        int damageProc = 0;             
        int addedTurnOnTrigger = 0;              
        float amplifierOnTrigger = 1.0f; 
    };
    
    struct DebuffStackRules {
        bool stackable = false;
        int maxStacks = 1;
        string maxStackEffectName; 
    };
    
    struct DebuffApplication {
        bool throwable = false;
        bool enchantOnWeapon = false; 
        string nameOnEnchantment;
        float effectChance = 1.0f; 
    };
};
struct ActiveDebuff {
    string name;
    int remainingDuration;
    int currentStacks;
};
class DebuffDatabase{
    public:
    struct DebuffDefinition {
        string name;
        debuffSettings::DebuffEffect effect;
        debuffSettings::DebuffStackRules stacking;
        debuffSettings::DebuffApplication application;
        string effectDesc;
    };
    const std::vector<DebuffDefinition>& getDebuffs() const {
        return debuffList;
    }
    private:
        std::vector<DebuffDefinition> debuffList{
           
             {"Burning",
            {10, 0, 0, 0, 0, 3, 5, 0, 1.0f},   // healthDecrease, attackDecrease, defenseDecrease, magicAttackDecrease, magicDefenseDecrease, durationTurns, damageProc, addedTurnOnTrigger, amplifierOnTrigger
            {true, 5, "Scorched"},              // stackable, maxStacks, maxStackEffectName
            {true, true, "Fire Aspect", 0.3f}, // throwable, enchantOnWeapon, nameOnEnchantment, effectChance
            "Deals 10 damage over 3 turns."
        },

  
        {"Freeze",
            {0, 0, 0, 0, 0, 3, 0, 2, 1.2f},    
            {true, 6, "Frostbite"},
            {true, true, "Frost Aspect", 1.0f},
            "Freezes the target, slowing them and dealing frost damage over time."
        },

       
        {"Poison",
            {5, 0, 0, 0, 0, 3, 0, 2, 1.3f},  
            {true, 6, "Envenomed"},
            {true, true, "Poisoned", 1.0f},
            "Poisons the target, dealing damage over time and stacking poison effects."
        },

       
        {"Bleeding",
            {5, 0, 0, 0, 0, 0, 0, 0, 2.0f}, 
            {true, 8, "Hemorrhage"},
            {true, true, "Vampiric", 1.0f},
            "Causes severe bleeding over time, dealing high damage and may heal the attacker."
        },


        {"Weakness",
            {0, 5, 0, 0, 0, 3, 0, 0, 1.0f},
            {true, 3, "Enfeebled"},
            {true, true, "Weakening Strike", 1.0f},
            "Reduces the target's attack temporarily."
        },

        {"Vulnerability",
            {0, 0, 0, 0, 0, 3, 0, 0, 1.0f}, 
            {true, 3, "Exposed"},
            {true, true, "Crippling Strike", 1.0f},
            "Makes the target more vulnerable, increasing damage taken."
        },

        {"Magic Dampening",
            {0, 0, 0, 5, 0, 3, 0, 0, 1.0f},
            {true, 3, "Arcane Suppressed"},
            {true, true, "Nullifying Strike", 1.0f},
            "Reduces the target's magic damage output."
        },

        {"Mana Weakness",
            {0, 0, 0, 0, 0, 3, 10, 0, 1.0f}, 
            {true, 3, "Mana Starved"},
            {true, true, "Mana Drain", 1.0f},
            "Reduces the target's mana and ability to cast spells."
        },

        {"Health Blight",
            {0, 0, 0, 0, 0, 3, 0, 0, 1.5f}, 
            {true, 3, "Blighted"},
            {true, true, "Blight Enchantment", 1.0f},
            "Reduces the target's max HP and increases all incoming damage."
        }
    };
};
class SpellDatabase {
public:
struct SpellData {
        std::string spellName;

        int manaCost;

        // Damage
        int healthDamage;
        int manaDamage;

        // Restoration
        int healthRestore;
        int manaRestore;

        // Buffs
        float armorIncrease;
        float magicArmorIncrease;

        // Debuffs
        float armorReduction;
        float magicArmorReduction;

        bool hasDebuff;
        std::vector<std::string> debuffs;
        float debuffModifierOnHit;

        // Weapon interaction
        bool applicableOnStaffs;
        float staffBonusModifier;

        int requiredLevel;
        std::string description;
        int pricePlatinum;
        int priceGold;
        int priceSilver;
        int priceCopper;
    };

    const std::vector<SpellData>& getSpells() const {
        return spellsList;
    }

private:
    std::vector<SpellData> spellsList{
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
};

class eventDatabase{
    public:
        struct eventProperties{
            string name;
            string description;
            int goldReward;
            int expReward;
            int healthEffect;
            int manaEffect;
            bool enemyEncounter;
            string enemyName;
            int enemyCount;

        };
        const std::vector<eventProperties>& getEvents() const {
            return eventList;
        }
    private:
        std::vector<eventProperties> eventList{
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
};

class locationDatabase{                                                                                                                                                                                                                                                                                                                                                                                                                  
    public:
        struct locationProperties{
            string name;
            string description;
            int difficultyLevel;
            LocationType type;
        };
        const std::vector<locationProperties>& getLocations() const {
            return locationList;
        }
    private:
        std::vector<locationProperties> locationList{
            {"Enchanted Forest", "A mystical forest filled with magical creatures.", 2, Terrain},
            {"Abandoned Castle", "An old castle rumored to be haunted by ghosts.", 4, Dungeon},
            {"Dark Caves", "A network of dark caves inhabited by dangerous monsters.", 5, Terrain},
            {"Abyssal Dungeon", "A deep dungeon descending into the abyss, filled with demonic horrors.", 4, Dungeon},
            {"Barren Plains", "Vast, desolate plains where the wind carries the screams of the damned.", 1, Terrain},
            {"Eldritch Forest", "A twisted forest where ancient evils lurk among gnarled trees.", 2, Terrain},
            {"Murky Swamp", "A foul swamp teeming with undead and poisonous mists.", 3, Terrain},
            {"Volcanic Wasteland", "A scorching land of lava and fire, home to fiery beasts.", 6, Terrain},
            {"Frozen Tundra", "An icy expanse where blizzards rage and ice monsters roam.", 3, Terrain},
            {"Poisoned Marshes", "A toxic bog filled with venomous creatures and deadly fumes.", 4, Terrain},
            {"Dragon's Lair", "A cavernous lair where a mighty dragon guards its hoard.", 7, Dungeon},
            {"Necromancer's Tower", "A towering spire where undead minions are raised.", 5, Dungeon},
            {"Haunted Ruins", "Ancient ruins cursed with spirits and traps.", 3, Dungeon},
            {"Shadowed Grove", "A dark grove where shadows come alive.", 2, Terrain},
            {"Cursed Citadel", "A fortress overrun by demonic forces.", 6, Dungeon},
            {"Wailing Caves", "Caves echoing with the cries of tormented souls.", 4, Terrain},
            {"Infernal Pit", "A bottomless pit leading to hellish realms.", 8, Dungeon},
            {"Peaceful Village", "A quiet village where people live in harmony.", 1, PeacefulVillage},
            {"Peaceful Town", "A bustling modest town.", 1, PeacefulTown},
            {"Witch Hut", "A mysterious hut where a witch sells spells.", 1, SpellStore},
            {"Enchanted Library", "A library filled with ancient tomes and magical knowledge.", 3, Dungeon},
            {"Forgotten Cemetery", "An overgrown graveyard where restless spirits roam.", 3, Terrain},
            {"Bloodstained Battlefield", "A scarred field littered with the remnants of ancient wars.", 4, Terrain},
            {"Mystic Ruins", "Crumbling ruins infused with arcane energy and hidden secrets.", 5, Dungeon},
            {"Crystal Caverns", "Glistening caves filled with crystal formations and echoing magic.", 4, Terrain},
            {"Thunderstorm Peaks", "Jagged mountains where lightning storms rage eternally.", 6, Terrain},
            {"Whispering Woods", "A forest where trees seem to whisper secrets and illusions abound.", 3, Terrain},
            {"Plague Lands", "A blighted region where disease and decay reign supreme.", 5, Terrain},
            {"Bandit Hideout", "A hidden camp where outlaws plot their next raids.", 3, Dungeon},
            {"Underwater Temple", "A submerged sanctuary guarded by aquatic horrors.", 7, Dungeon},
            {"Floating Islands", "Islands suspended in the sky, home to aerial creatures.", 5, Terrain},
            {"Desert Oasis", "A lush paradise in the midst of endless sands, hiding ancient treasures.", 2, Terrain},
            {"Lava Fields", "Fields of molten rock where fire elementals dance.", 6, Terrain},
            {"Ice Palace", "A grand palace of ice, ruled by frost queens.", 5, Dungeon},
            {"Ghost Town", "An abandoned settlement haunted by the echoes of its past inhabitants.", 2, Terrain},
            {"Arcane Academy", "A prestigious school of magic, filled with aspiring sorcerers.", 3, SpellStore},
            {"Pirate Cove", "A hidden cove where pirates hoard their ill-gotten gains.", 4, Dungeon},
            {"Eternal Flame Volcano", "A volcano with an unquenchable fire at its heart.", 7, Terrain},
            {"Moonlit Glade", "A serene clearing bathed in eternal moonlight, home to fey creatures.", 2, Terrain},
            {"Abyssal Trench", "A deep oceanic trench where eldritch horrors dwell.", 8, Terrain},

        };
};

class NPCGenerator{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
    public:
    NPCGenerator() : firstNames({"Godfrey","Augustus","Edmund","Alfred","Theodore","Marcus","Julius","Lucius","Constantine","Benedict","Sebastian","Cornelius","Ambrose","Gregory","Leonard","Matthias","Philip","Alexander","Dominic","Victor","Hugh","Gerald","Roland","Bernard","Percival"}),
                     lastNames({"Janus","Godwin","Edwards","Aurelian","Constantinus","Benedictus","Marcellus","Valerian","Justinian","Hadrian","Maximus","Cassius","Severus","Flavian","Gratian","Laurentius","Paulinus","Victorinus","Dominicus","Magnus","Theodoric","Regulus","Claudius","Tiberius","Germanicus"}),
                     specialCharacters({"Evelyn Chevalier", "Astra Yao", "Ye Shunguang ", "Burnice White","Jane Doe", "Belle"}) {}

    void lockName(const string& name) {
        lockedNames.insert(name);
    }

    void unlockName(const string& name) {
        lockedNames.erase(name);
    }

    NPC generateNPC(int playerLevel) {
        static std::mt19937 gen(std::random_device{}());

       
        int minLevel = std::max(1, playerLevel - 2);
        int maxLevel = playerLevel + 2;
        std::uniform_int_distribution<int> levelDist(minLevel, maxLevel);
        int level = levelDist(gen);

        std::uniform_real_distribution<float> specialDist(0.0f, 1.0f);
        bool isSpecial = specialDist(gen) < 0.1f; // 10% chance

        string name;
        if (isSpecial) {
            std::uniform_int_distribution<size_t> specialCharDist(0, specialCharacters.size() - 1);
            name = specialCharacters[specialCharDist(gen)];
        } else {
       
            do {
                std::uniform_int_distribution<size_t> firstDist(0, firstNames.size() - 1);
                std::uniform_int_distribution<size_t> lastDist(0, lastNames.size() - 1);
                name = firstNames[firstDist(gen)] + " " + lastNames[lastDist(gen)];
            } while (lockedNames.count(name));
        }

  
        lockedNames.insert(name);

      
        PlayerRaceDatabase raceDB;
        const auto& races = raceDB.templates;
        std::uniform_int_distribution<size_t> raceDist(0, races.size() - 1);
        size_t raceIdx = raceDist(gen);
        PlayerRaceTemplate chosenRace = races[raceIdx];
        PlayerRace npcRace{chosenRace.name, {chosenRace.lore.description}};

        // Random class
        PlayerClassCollection classDB;
        const auto& classes = classDB.templates;
        std::uniform_int_distribution<size_t> classDist(0, classes.size() - 1);
        size_t classIdx = classDist(gen);
        PlayerClassTemplate chosenClass = classes[classIdx];

      
        NPC npc;
        npc.name = name;
        npc.race = npcRace;
        npc.playerClass = chosenClass;
        npc.level = level;
        npc.stats = chosenClass.stats;

       
        npc.stats.hitpoints += chosenRace.statBonus.hitpoints;
        npc.stats.maxHitpoints += chosenRace.statBonus.maxHitpoints;
        npc.stats.armor += chosenRace.statBonus.armor;
        npc.stats.magicArmor += chosenRace.statBonus.magicArmor;
        npc.stats.attack += chosenRace.statBonus.attack;
        npc.stats.magicAttack += chosenRace.statBonus.magicAttack;
        npc.stats.mana += chosenRace.statBonus.mana;
        npc.stats.maxMana += chosenRace.statBonus.maxMana;

       
        float scale = 1.0f + (level - 1) * 0.1f;
        npc.stats.scale(scale);

    
        EquipmentandWeaponDatabase eqDB;
        const auto& equipment = eqDB.getEquipment();
        std::uniform_int_distribution<size_t> eqDist(0, equipment.size() - 1);
        size_t eqIdx = eqDist(gen);
        const auto& eq = equipment[eqIdx];
        if (eq.type == "Weapon") {
            npc.equippedWeapon = eq.name;
            npc.stats.attack += eq.attackIncrease;
            npc.stats.magicAttack += eq.magicAttackIncrease;
        } else {
            npc.equippedArmor = eq.name;
            npc.stats.armor += eq.defenseIncrease;
            npc.stats.magicArmor += eq.magicDefenseIncrease;
        }

  
        npc.inventory = {"Small Health Potion", "Small Health Potion", "Small Health Potion", "Small Mana Potion", "Small Mana Potion", "Small Mana Potion"};

   
        int baseWage = 10 + level * 2;
        npc.wagePerWeek = isSpecial ? baseWage * 2 : baseWage;



       
       std::vector<string> stories = {
            "I arrived here following the migration of the birds. I figured if they were heading south to find something better, I might as well see where the path led me. I haven't found a reason to turn back yet.",
            "A heavy flood took my family's land three summers ago. With nothing left to pull from the mud, I packed what could fit in a single satchel and started walking. This was the first place that felt solid enough to stop.",
            "I was part of a caravan that got caught in a mountain pass during a blizzard. By the time the snow cleared, the group had split up. I took the downhill trail, and it eventually opened up into the gates of this place.",
            "To be honest, I left my home because the silence was getting too loud. Every day was exactly like the one before it, and I realized if I didn't leave then, I would eventually become part of the scenery. So, I walked.",
            "I followed a map I found tucked inside an old book. It didn't lead to gold or glory like I'd hoped—just a long, winding road that ended right here. Perhaps the map wasn't about the destination, but just the exit.",
            "My journey started with a simple disagreement with the local magistrate. It was easier to vanish into the night than to stay and argue a point that no one wanted to hear. I've been a stranger in new lands ever since.",
            "I came here looking for someone I used to know. I've since realized they are long gone, but by the time I figured that out, I had already grown fond of the way the morning sun hits the buildings in this square.",
            "The wind seemed to push me here. Every time I reached a crossroads and felt uncertain, a gust would pick up from the north or the west, nudging me along. I stopped here because, for once, the air went still.",
            "I grew up on the back of a wagon, always moving. When I finally reached this place, my boots were worn through and my spirit was tired. I decided then that I was done with the road for a while.",
            "I left home when the inheritance was divided and there was nothing left for the youngest but a sturdy pair of shoes and a blessing. I’ve used the shoes well, and I suppose the blessing brought me here safely.",
            "There was a fire in my old village—not a raid, just a stray spark on a dry night. Most of us scattered to the neighboring valleys. I kept going a bit further than the rest, wanting a fresh start where no one knew my name.",
            "I was traveling with a troupe of performers until our wagon broke an axle ten miles back. They stayed to fix it, but I decided to keep walking. I think I was just ready to see the world at my own pace.",
            "I came here by sea, tucked away in the corner of a merchant's hull. The salt air was all I knew for a month, and when I finally stepped onto the docks here, the ground felt so steady I didn't want to leave it.",
            "I spent years looking for a specific tree I saw in a dream as a child. I haven't found the tree yet, but the path I took trying to find it brought me through these gates, and it felt right to stay.",
            "I simply walked out of my front door one morning and forgot to turn around. By the time I realized how far I’d gone, the horizon behind me looked just as unfamiliar as the one ahead. So, I kept going until I hit this spot."
        };
        std::uniform_int_distribution<size_t> storyDist(0, stories.size() - 1);
        npc.story = stories[storyDist(gen)];

    
        SpellDatabase spellDB;
        const auto& spells = spellDB.getSpells();
        std::vector<string> availableSpells;
        for (const auto& spell : spells) {
            if (spell.requiredLevel <= level) {
                availableSpells.push_back(spell.spellName);
            }
        }
        int numSpells = std::min(2, (int)availableSpells.size());
        std::shuffle(availableSpells.begin(), availableSpells.end(), gen);
        for (int i = 0; i < numSpells; ++i) {
            npc.spells.push_back(availableSpells[i]);
        }

        // Assign dialogues
        if (isSpecial) {
            if (name == "Evelyn Chevalier") {
                npc.dialogues = {"It'll be my duty to ensure your safety.",
                                "Hm? It's just me you need, and not a bodyguard...? I see.",
                                "Have you noticed? People on the street may all appear to be similar, but they actually have subtle differences.",
                                "Don't worry. I'm right behind you.",
                                "What's the matter? Something on your mind?", 
                                "I'm used to people staring at me while I'm working."
                            };
            } else if (name == "Astra Yao") {
                npc.dialogues = {"There's no need for disguises. We're heading out into the world with confidence!",
                                "Want me to hold your hand? I don't care if anyone sees.",
                                "As long as you're just daydreaming...",
                                "I want to find somewhere to sit down...",
                                "Have you decided where we're going next? Time is of the essence."
                            };
            } else if (name == "Ye Shunguang ") {
                npc.dialogues = {"If you ever want to go out for a walk, just let me know!",
                                "You're here! Shall we head out now?",
                                "The weather's always so nice when I'm out with you~",
                                "Where'd you wanna go, Junior? I'll follow your lead.",
                                "Stop spacing out... Come on, take my hand. Let's keep going.",
                                "If only time could stop at this moment..."
                            };
            } else if (name == "Burnice White") {
                npc.dialogues = {"Hey, you lookin' for me? C'mon, who else would you be lookin' for?!",
                                "Ooh! Where we going? Oh, you should show me around your neighborhood!",
                                "Hm? Did something spark your interest?",
                                "What's up? See someone you know?",
                                "And even if there were crumbs, they would've been blown away on the ride here!",
                                "You're not... getting tired already, are ya?"
                            };
            } else if (name == "Jane Doe") {
                npc.dialogues = {" Hey there~ It's been a while. I've been looking forward to seeing you.",
                                "So, where'd you wanna go? I'm down for anything~",
                                "Hey, you're not moving... \n Are you pretending to be a statue?",
                                "Feeling tired already? I can carry you the rest of the way~",
                                "If you keep staring at me like that, I might actually get embarrassed... \nJust kidding~",
                                "Why are you staring at me? Do I have something on my face?"};
            } else if (name == "Belle") {
                npc.dialogues = {"So sleepy... Brain feels so foggy... Coffee... I need coffee....",
                                "Wouldn't it be weird if I'm a vampire and you are human? ,I might bite you first... Wait, did you turn me into a vampire? when i was sleeping?",
                                "I just realized I haven't played guitar for a long time.",
                                "Is it really all up to me? I'm so glad you trust me with running the party! I will not let you down!",
                                "Why do I crave Caramel Boba Oolong Teamilk in the middle of this quiet day?",
                                "Makes me wonder why I even bother with trying to save the money. You don't feel this way?"};
            }
        } else {
            std::vector<string> normalDialogues = {
                "The air feels a bit heavy today. I wouldn't be surprised if we saw a bit of rain before sunset.",
                "It's a quiet afternoon, isn't it? Sometimes the silence is exactly what a person needs.",
                "I was just thinking about how much the light changes the look of these old stone walls.",
                "Have you been this way before? It feels like every time I turn a corner, I notice something new.",
                "The days seem to be getting shorter. It'll be dark before we know it.",
                "I wonder what lies beyond those hills. Most people are content to stay put, but I always find myself looking outward.",
                "It's nice to see a friendly face. This place can feel a bit hollow when the crowds thin out.",
                "I heard a bird singing earlier that I didn't recognize. It's funny how the small things stick with you.",
                "Do you ever get the feeling that time moves slower in some places than it does in others?",
                "I was just resting my eyes for a moment. It's been a long day, even if I haven't done much.",
                "There's something comforting about the smell of woodsmoke in the evening, don't you think?",
                "I've spent a lot of time just watching people pass by. Everyone seems to be in such a hurry to get somewhere.",
                "The ground is a bit uneven here. You'd think they would have leveled these stones centuries ago.",
                "I'm not looking for much just a bit of shade and a moment to catch my breath.",
                "If you're heading further down the road, keep an eye on the clouds. They look like they're shifting."
            };
            std::shuffle(normalDialogues.begin(), normalDialogues.end(), gen);
            int num = 2 + (gen() % 2);
            for (int i = 0; i < num; ++i) {
                npc.dialogues.push_back(normalDialogues[i]);
            }
        }

        return npc;
    }

    private:
    const std::vector<string> firstNames;
    const std::vector<string> lastNames;
    const std::vector<string> specialCharacters;
    std::set<string> lockedNames;
};

class PlayerInventory {
public:
    enum class ItemType { Potion, Equipment, FoodAndDrink };

    struct InventoryItem {
        ItemType type;
        size_t dbIndex;
        int quantity;
        bool enchanted = false;
    };


    void addItem(ItemType type, size_t dbIndex, int amount = 1) {
        for (auto& item : inventory) {
            if (item.type == type && item.dbIndex == dbIndex) {
                item.quantity += amount;
                return;
            }
        }
        inventory.push_back({ type, dbIndex, amount });
    }


    void showInventory(Player& player) {
        if (inventory.empty()) {
            cout << "Inventory is empty.\n";
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return;
        }

        std::vector<string> lines;
        for (size_t i = 0; i < inventory.size(); ++i) {
            string itemLine = std::to_string(i + 1) + ". " + getItemName(inventory[i]);
            if (inventory[i].type == ItemType::Equipment && isEquipped(inventory[i].dbIndex)) {
                itemLine += " (Equipped";
                if (inventory[i].enchanted) {
                    itemLine += ", Enchanted";
                }
                itemLine += ")";
            }
            if (inventory[i].enchanted && !isEquipped(inventory[i].dbIndex)) {
                itemLine += " [Enchanted]";
            }
            itemLine += " x" + std::to_string(inventory[i].quantity);
            lines.push_back(itemLine);
        }

        displayBorderedMenu(lines, "Choose item (0 to exit): ");
        int choice;
        cin >> choice;
        cin.ignore();

        if (choice <= 0 || choice > static_cast<int>(inventory.size())) return;

        handleSelectedItem(player, inventory[static_cast<size_t>(choice - 1)]);
    }

    void tickBuffs(Player& player) {
        if (!activeBuff) return;

        activeBuff->remainingTurns--;
        if (activeBuff->remainingTurns <= 0) {
            removeBuff(player);
            cout << activeBuff->name << " has worn off.\n";
            activeBuff.reset();
        }
    }

private:
    struct Buff {
        string name;
        int attackBonus = 0;
        int defenseBonus = 0;
        int magicAttackBonus = 0;
        int magicDefenseBonus = 0;
        int maxManaBonus = 0;
        float critRateBonus = 0.0f;
        float critDamageBonus = 0.0f;
        int remainingTurns = 0;

        Buff(string n, int ab, int db, int mab, int mdb, int mmb, float crb, float cdb, int rt)
            : name(n), attackBonus(ab), defenseBonus(db), magicAttackBonus(mab), magicDefenseBonus(mdb), maxManaBonus(mmb), critRateBonus(crb), critDamageBonus(cdb), remainingTurns(rt) {}
    };

public:
    const std::optional<Buff>& getActiveBuff() const {
        return activeBuff;
    }

    struct EquippedSlots {
        int armorIndex = -1;
        int weaponIndex = -1;
        int staffIndex = -1;
    };

    std::vector<InventoryItem> inventory;
    EquippedSlots equipped;
    std::optional<Buff> activeBuff;

    PotionDatabase potionDB;
    EquipmentandWeaponDatabase equipmentDB;
    FoodandDrinksDatabase foodDB;
    SpellDatabase spellDB;

    string getItemName(const InventoryItem& item) const {
        if (item.type == ItemType::Potion)
            return potionDB.getPotions()[item.dbIndex].name;
        else if (item.type == ItemType::Equipment)
            return equipmentDB.getEquipment()[item.dbIndex].name;
        else if (item.type == ItemType::FoodAndDrink)
            return foodDB.getFoodAndDrink()[item.dbIndex].name;
        else
            return "Unknown Item";
    }

    bool isWeapon(const EquipmentandWeaponDatabase::equipmentProperties& eq) const {
        return eq.attackIncrease > 0 || eq.magicAttackIncrease > 0;
    }

    bool isEquipped(size_t index) const {
        return equipped.armorIndex == static_cast<int>(index) ||
               equipped.weaponIndex == static_cast<int>(index) ||
               equipped.staffIndex == static_cast<int>(index);
    }

    void handleSelectedItem(Player& player, InventoryItem& item) {
        if (item.type == ItemType::Potion)
            usePotion(player, item);
        else if (item.type == ItemType::FoodAndDrink)
            useFoodAndDrink(player, item);
        else
            equipItem(player, item);
    }



    void usePotion(Player& player, InventoryItem& item) {
        const auto& potion = potionDB.getPotions()[item.dbIndex];

        cout << "Use " << potion.name << "? (y/n): ";
        char c; cin >> c; cin.ignore();
        if (c != 'y') return;

    
        if (potion.hpEffect > 0) {
            int heal = std::min(potion.hpEffect, player.stats.maxHitpoints - player.stats.hitpoints);
            player.stats.hitpoints += heal;
            cout << "Recovered " << heal << " HP.\n";
        }

        
        if (potion.manaRestore > 0) {
            int restore = std::min(potion.manaRestore, player.stats.maxMana - player.stats.mana);
            player.stats.mana += restore;
            cout << "Recovered " << restore << " Mana.\n";
        }

     
        if (potion.durationTurns > 0) {
            removeBuff(player);

            activeBuff = Buff{
                potion.name,
                potion.attackEffect,
                potion.defenseEffect,
                potion.magicEffect,
                potion.magicDefenseEffect,
                potion.manaIncreaseMax,
                potion.critRateEffect,
                potion.critDamageEffect,
                potion.durationTurns
            };

            player.stats.attack += potion.attackEffect;
            player.stats.armor += static_cast<float>(potion.defenseEffect);
            player.stats.magicAttack += potion.magicEffect;
            player.stats.magicArmor += static_cast<float>(potion.magicDefenseEffect);
            player.stats.critRate += potion.critRateEffect;
            player.stats.critDamage += potion.critDamageEffect;
            if (potion.manaIncreaseMax > 0) {
                player.stats.maxMana += potion.manaIncreaseMax;
                player.stats.mana += potion.manaIncreaseMax;
            }

            cout << potion.effectDesc << endl;
        }

        item.quantity--;
        cleanupInventory();
    }

    void useFoodAndDrink(Player& player, InventoryItem& item) {
        const auto& food = foodDB.getFoodAndDrink()[item.dbIndex];

        cout << "Use " << food.name << "? (y/n): ";
        char c; cin >> c; cin.ignore();
        if (c != 'y') return;

    
        if (food.healthRestoration > 0) {
            int heal = std::min(food.healthRestoration, player.stats.maxHitpoints - player.stats.hitpoints);
            player.stats.hitpoints += heal;
            cout << "Recovered " << heal << " HP.\n";
        }

     
        if (food.manaRestoration > 0) {
            int restore = std::min(food.manaRestoration, player.stats.maxMana - player.stats.mana);
            player.stats.mana += restore;
            cout << "Recovered " << restore << " Mana.\n";
        }

   
        if (food.maxHealthBuffBonus > 0) {
            player.stats.maxHitpoints += food.maxHealthBuffBonus;
            cout << "Max HP increased by " << food.maxHealthBuffBonus << ".\n";
        }

        if (food.maxManaBuffBonus > 0) {
            player.stats.maxMana += food.maxManaBuffBonus;
            cout << "Max Mana increased by " << food.maxManaBuffBonus << ".\n";
        }

        item.quantity--;
        cleanupInventory();
    }

    void equipItem(Player& player, InventoryItem& item) {
        const auto& eq = equipmentDB.getEquipment()[item.dbIndex];
        bool weapon = isWeapon(eq);
        bool staff = eq.type == "Staff";
        bool alreadyEquipped = isEquipped(item.dbIndex);

        if (alreadyEquipped) {
            cout << "Unequip " << eq.name << "? (y/n): ";
            char c; cin >> c; cin.ignore();
            if (c != 'y') return;

            if (staff) unequipStaff(player);
            else if (weapon) unequipWeapon(player);
            else unequipArmor(player);
            cout << eq.name << " unequipped.\n";
            return;
        }

   
        cout << "Equip " << eq.name << "? (y/n): ";
        char c; cin >> c; cin.ignore();
        if (c != 'y') return;

        if (staff) equipStaff(player, item.dbIndex);
        else if (weapon) equipWeapon(player, item.dbIndex);
        else equipArmor(player, item.dbIndex);

    
        auto it = std::find_if(inventory.begin(), inventory.end(), [&](const InventoryItem& i){ return &i == &item; });
        if (it != inventory.end()) moveItemToTop(std::distance(inventory.begin(), it));

        cout << eq.effectDesc << endl;
    }

    void cleanupInventory() {
        inventory.erase(
            std::remove_if(inventory.begin(), inventory.end(), [](const InventoryItem& i){ return i.quantity <= 0; }),
            inventory.end()
        );
    }

    void removeBuff(Player& player) {
        if (!activeBuff) return;
        player.stats.attack -= activeBuff->attackBonus;
        player.stats.armor -= static_cast<float>(activeBuff->defenseBonus);
        player.stats.maxMana -= activeBuff->maxManaBonus;
        player.stats.critRate -= activeBuff->critRateBonus;
        player.stats.critDamage -= activeBuff->critDamageBonus;
        if (player.stats.mana > player.stats.maxMana) {
            player.stats.mana = player.stats.maxMana;
        }
    }

   

    void equipArmor(Player& player, size_t index) {
        const auto& eq = equipmentDB.getEquipment()[index];
        player.stats.armor += static_cast<float>(eq.defenseIncrease);
        player.stats.magicArmor += static_cast<float>(eq.magicDefenseIncrease);
        equipped.armorIndex = static_cast<int>(index);
    }

    void equipWeapon(Player& player, size_t index) {
        const auto& eq = equipmentDB.getEquipment()[index];
        player.stats.attack += eq.attackIncrease;
        player.stats.magicAttack += eq.magicAttackIncrease;
        player.equippedWeaponDebuffs = eq.debuffs;
        player.equippedWeaponDebuffChance = eq.debuffChance;
        equipped.weaponIndex = static_cast<int>(index);
    }

    void unequipArmor(Player& player) {
        if (equipped.armorIndex == -1) return;
        const auto& eq = equipmentDB.getEquipment()[equipped.armorIndex];
        player.stats.armor -= static_cast<float>(eq.defenseIncrease);
        player.stats.magicArmor -= static_cast<float>(eq.magicDefenseIncrease);
        equipped.armorIndex = -1;
    }

    void unequipWeapon(Player& player) {
        if (equipped.weaponIndex == -1) return;
        const auto& eq = equipmentDB.getEquipment()[equipped.weaponIndex];
        player.stats.attack -= eq.attackIncrease;
        player.stats.magicAttack -= eq.magicAttackIncrease;
        player.equippedWeaponDebuffs.clear();
        player.equippedWeaponDebuffChance = 0.0f;
        equipped.weaponIndex = -1;
    }

    void equipStaff(Player& player, size_t index) {
        const auto& eq = equipmentDB.getEquipment()[index];
        player.stats.magicAttack += eq.magicAttackIncrease;
        equipped.staffIndex = static_cast<int>(index);

  
        for (const auto& spellName : player.equippedStaffSpells) {
            const auto& spells = spellDB.getSpells();
            auto it = std::find_if(spells.begin(), spells.end(), [&](const SpellDatabase::SpellData& s){ return s.spellName == spellName; });
            if (it != spells.end()) {
                const auto& spell = *it;
                player.stats.magicAttack += static_cast<int>(std::round(spell.healthDamage * spell.staffBonusModifier));
            }
        }
    }

    void unequipStaff(Player& player) {
        if (equipped.staffIndex == -1) return;
        const auto& eq = equipmentDB.getEquipment()[equipped.staffIndex];
        player.stats.magicAttack -= eq.magicAttackIncrease;

     
        for (const auto& spellName : player.equippedStaffSpells) {
            const auto& spells = spellDB.getSpells();
            auto it = std::find_if(spells.begin(), spells.end(), [&](const SpellDatabase::SpellData& s){ return s.spellName == spellName; });
            if (it != spells.end()) {
                const auto& spell = *it;
                player.stats.magicAttack -= static_cast<int>(std::round(spell.healthDamage * spell.staffBonusModifier));
            }
        }

        equipped.staffIndex = -1;
    }

    void moveItemToTop(size_t index) {
        if (index == 0 || index >= inventory.size()) return;
        InventoryItem item = inventory[index];
        inventory.erase(inventory.begin() + static_cast<int>(index));
        inventory.insert(inventory.begin(), item);
    }
};

class PlayerController {
public:
    PlayerController(Player& p, PlayerInventory& inv)
        : player(p), inventory(inv), reqAmount(100) {}

    void showStats() const {
        cout << "\n=== Player Stats ===" << endl;
        cout << "Hitpoints: " << player.stats.hitpoints << "/" << player.stats.maxHitpoints << endl;

        auto activeBuff = inventory.getActiveBuff();
        if (activeBuff) {
            cout << "Armor: " << player.stats.armor
                 << "( + " << activeBuff->defenseBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << endl;
            cout << "Magic Armor: " << player.stats.magicArmor
                 << "( + " << activeBuff->magicDefenseBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << endl;
            cout << "Attack: " << player.stats.attack
                 << "( + " << activeBuff->attackBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << endl;
            cout << "Magic Attack: " << player.stats.magicAttack
                 << "( + " << activeBuff->magicAttackBonus << " from a potion, (" << activeBuff->remainingTurns << " turns left))" << endl;
        } else {
            cout << "Armor: " << player.stats.armor << endl;
            cout << "Magic Armor: " << player.stats.magicArmor << endl;
            cout << "Attack: " << player.stats.attack << endl;
            cout << "Magic Attack: " << player.stats.magicAttack << endl;
        }

        cout << "Level: " << player.stats.level << endl;
        cout << "Experience: " << player.stats.expe << "/" << reqAmount << endl;
        cout << "Currency: " << player.economy.platinum << "p " << player.economy.gold << "g " << player.economy.silver << "s " << player.economy.copper << "c" << endl;
        cout << "\nPress Enter to continue...";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    void levelUpChecker() {
        while (player.stats.expe >= reqAmount) {
            player.stats.expe -= reqAmount;
            player.stats.level += 1;


            player.stats.scale(1.1f);


            reqAmount *= 1.2f;

            cout << "You leveled Up!" << endl;
            cout << "Your stats have been increased by 10%." << endl;
        }
    }

private:
    Player& player;
    PlayerInventory& inventory;
    float reqAmount;
};

struct EnemyStats {
    struct StatsData {
        int hitpoints;
        int maxHitpoints;
        float armor;
        float magicArmor;
        int attack;
        int magicAttack;
        int mana;
        int maxMana;
        float critRate;
        float critDamage;
        float dodgeRate;
        float magicAmplifierRate;
        int level;
        float expe;
        Economy economy;
    } data;

    bool scaled = false;

    void scale(float multiplier) {
        data.maxHitpoints = static_cast<int>(std::round(data.maxHitpoints * multiplier));
        data.maxMana = static_cast<int>(std::round(data.maxMana * multiplier));
        data.hitpoints = static_cast<int>(std::round(data.hitpoints * multiplier));
        data.hitpoints = std::min<int>(data.hitpoints, data.maxHitpoints);
        data.mana = static_cast<int>(std::round(data.mana * multiplier));
        data.mana = std::min<int>(data.mana, data.maxMana);
        data.armor *= multiplier;
        data.magicArmor *= multiplier;
        data.attack = static_cast<int>(std::round(data.attack * multiplier));
        data.magicAttack = static_cast<int>(std::round(data.magicAttack * multiplier));
        data.dodgeRate *= multiplier;
        data.critRate *= multiplier;
        data.critDamage *= multiplier;
        data.magicAmplifierRate *= multiplier;
    }
};

struct enemyLore {
    string desc;
};

struct Enemy {
    string name;
    EnemyStats stats;
    std::vector<string> debuffs;
};

struct EnemyDatabase {
struct EnemyTemplate {
        string name;
        EnemyStats stats;
        enemyLore lore;
        int minLevel;
        std::vector<std::string> debuffs;
        int difficultyLevel;
        std::vector<LocationType> allowedLocations;
    };
    std::vector<EnemyTemplate> templates = {
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
    Enemy getRandomEnemy(int difficultyLevel, LocationType locationType) {
        static std::mt19937 gen(std::random_device{}());
        std::vector<const EnemyTemplate*> validEnemies;

        for (const auto& tmpl : templates) {
            if (tmpl.difficultyLevel <= difficultyLevel &&
                std::find(tmpl.allowedLocations.begin(), tmpl.allowedLocations.end(), locationType) != tmpl.allowedLocations.end()) {
                validEnemies.push_back(&tmpl);
            }
        }

        if (validEnemies.empty()) {
            throw std::runtime_error("No valid enemies for this difficulty level and location type!");
        }

        std::uniform_int_distribution<size_t> dist(0, validEnemies.size() - 1);
        const EnemyTemplate* chosen = validEnemies[dist(gen)];

        return Enemy{ chosen->name, chosen->stats, chosen->debuffs };
    }
    Enemy getRandomEnemy(int difficultyLevel) {
        static std::mt19937 gen(std::random_device{}());
        std::vector<const EnemyTemplate*> validEnemies;

        for (const auto& tmpl : templates) {
            if (tmpl.difficultyLevel <= difficultyLevel) {
                validEnemies.push_back(&tmpl);
            }
        }

        if (validEnemies.empty()) {
            throw std::runtime_error("No valid enemies for this difficulty level!");
        }

        std::uniform_int_distribution<size_t> dist(0, validEnemies.size() - 1);
        const EnemyTemplate* chosen = validEnemies[dist(gen)];

        return Enemy{ chosen->name, chosen->stats, chosen->debuffs };
    }
};

class EnemyController {
public:
   
    Enemy encounterEnemy(int difficultyLevel, LocationType locationType = Terrain) {
        Enemy enemy = enemyDB.getRandomEnemy(difficultyLevel, locationType);
        enemyScaleLevel(enemy, difficultyLevel, locationType);

        cout << "A wild " << enemy.name << " has appeared!" << endl;
        return enemy;
    }

    Enemy getEnemyByName(const string& name, int playerLevel, LocationType locationType = Terrain) {
        for (const auto& tmpl : enemyDB.templates) {
            if (tmpl.name == name) {
                Enemy enemy{ tmpl.name, tmpl.stats, tmpl.debuffs };
                enemyScaleLevel(enemy, playerLevel, locationType);
                return enemy;
            }
        }
        throw std::runtime_error("Enemy not found: " + name);
    }

    void enemyGoldExpDrop(Player& player, const Enemy& enemy) {
        player.economy.addCurrency(enemy.stats.data.economy.platinum, enemy.stats.data.economy.gold, enemy.stats.data.economy.silver, enemy.stats.data.economy.copper);
        player.stats.expe += enemy.stats.data.expe;

        cout << "You have defeated " << enemy.name
             << ", you gained " << enemy.stats.data.economy.platinum << "p " << enemy.stats.data.economy.gold << "g " << enemy.stats.data.economy.silver << "s " << enemy.stats.data.economy.copper << "c"
             << " and " << enemy.stats.data.expe
             << " experience!" << endl;

        if (player.defeatedEnemies.find(enemy.name) == player.defeatedEnemies.end()) {
            player.defeatedEnemies.insert(enemy.name);
            player.hasNewDictionaryEntry = true;
        }
    }

private:
    EnemyDatabase enemyDB;

    void enemyScaleLevel(Enemy& enemy, int difficultyLevel, LocationType locationType = Terrain) {
        if (enemy.stats.scaled || enemy.stats.data.level >= difficultyLevel)
            return;

        float scale = 1.0f + static_cast<float>(difficultyLevel - enemy.stats.data.level) * 0.1f;
        if (scale < 0.5f) scale = 0.5f;

        enemy.stats.scale(scale);

        float locationMultiplier = 1.0f;
        switch (locationType) {
            case PeacefulVillage: locationMultiplier = 0.8f; break;
            case PeacefulTown: locationMultiplier = 0.9f; break;
            case Terrain: locationMultiplier = 1.0f; break;
            case Dungeon: locationMultiplier = 1.2f; break;
            case SpellStore: locationMultiplier = 0.9f; break;
        }
        enemy.stats.scale(locationMultiplier);

        switch (locationType) {
            case PeacefulVillage:
                enemy.stats.data.dodgeRate += 0.05f;
                enemy.stats.data.attack = std::max(0, enemy.stats.data.attack - 2);
                enemy.stats.data.magicAttack = std::max(0, enemy.stats.data.magicAttack - 2);
                break;
            case PeacefulTown:
                enemy.stats.data.dodgeRate += 0.03f;
                enemy.stats.data.attack = std::max(0, enemy.stats.data.attack - 1);
                enemy.stats.data.magicAttack = std::max(0, enemy.stats.data.magicAttack - 1);
                break;
            case Terrain:
                break;
            case Dungeon:
                enemy.stats.data.magicArmor += 0.1f;
                enemy.stats.data.magicAttack += 5;
                enemy.stats.data.dodgeRate = std::max(0.0f, enemy.stats.data.dodgeRate - 0.05f);
                break;
            case SpellStore:
                break;
        }

        enemy.stats.data.level = difficultyLevel;
        enemy.stats.scaled = true;
    }
};

struct CombatResult {
    string attackerName;
    string targetName;
    int physicalDamage;
    int magicalDamage;
    int totalDamage;
    bool isCrit;
    bool dodge;
    string debuffInflicted;
};

class ICombatant {
public:
    virtual string getName() const = 0;
    virtual int getPhysicalAttack() const = 0;
    virtual int getMagicAttack() const = 0;
    virtual float getArmor() const = 0;
    virtual float getMagicArmor() const = 0;
    virtual float getCritRate() const = 0;
    virtual float getCritDamage() const = 0;
    virtual float getDodgeRate() const = 0;
    virtual bool isPlayer() const = 0;
    virtual void takeDamage(int amount) = 0;
    virtual const std::vector<string>& getDebuffs() const = 0;
    virtual void applyDebuff(const string& debuff) = 0;
    virtual ~ICombatant() = default;
};

class NPCCombatant : public ICombatant {
public:
    NPCCombatant(NPC& n) : npc(n) {}
    string getName() const override { return npc.name; }
    int getPhysicalAttack() const override { return npc.stats.attack; }
    int getMagicAttack() const override { return npc.stats.magicAttack; }
    float getArmor() const override { return npc.stats.armor; }
    float getMagicArmor() const override { return npc.stats.magicArmor; }
    float getCritRate() const override { return npc.stats.critRate; }
    float getCritDamage() const override { return npc.stats.critDamage; }
    float getDodgeRate() const override { return npc.stats.dodgeRate; }
    bool isPlayer() const override { return false; }
    void takeDamage(int amount) override { npc.stats.hitpoints -= amount; }
    const std::vector<string>& getDebuffs() const override { return npc.debuffs; }
    void applyDebuff(const string& debuff) override { npc.debuffs.push_back(debuff); }

private:
    NPC& npc;
};

class PlayerCombatant : public ICombatant {
public:
    PlayerCombatant(Player& p) : player(p) {}
    string getName() const override { return player.name; }
    int getPhysicalAttack() const override { return player.stats.attack; }
    int getMagicAttack() const override { return player.stats.magicAttack; }
    float getArmor() const override { return player.stats.armor; }
    float getMagicArmor() const override { return player.stats.magicArmor; }
    float getCritRate() const override { return player.stats.critRate; }
    float getCritDamage() const override { return player.stats.critDamage; }
    float getDodgeRate() const override { return player.stats.dodgeRate; }
    bool isPlayer() const override { return true; }
    void takeDamage(int amount) override { player.stats.hitpoints -= amount; }
    const std::vector<string>& getDebuffs() const override { return player.debuffs; }
    void applyDebuff(const string& debuff) override { player.debuffs.push_back(debuff); }
    const std::vector<string>& getEquippedWeaponDebuffs() const { return player.equippedWeaponDebuffs; }
    float getEquippedWeaponDebuffChance() const { return player.equippedWeaponDebuffChance; }

private:
    Player& player;
};

class EnemyCombatant : public ICombatant {
public:
    EnemyCombatant(Enemy& e) : enemy(e) {}
    string getName() const override { return enemy.name; }
    int getPhysicalAttack() const override { return enemy.stats.data.attack; }
    int getMagicAttack() const override { return enemy.stats.data.magicAttack; }
    float getArmor() const override { return enemy.stats.data.armor; }
    float getMagicArmor() const override { return enemy.stats.data.magicArmor; }
    float getCritRate() const override { return enemy.stats.data.critRate; }
    float getCritDamage() const override { return enemy.stats.data.critDamage; }
    float getDodgeRate() const override { return enemy.stats.data.dodgeRate; }
    bool isPlayer() const override { return false; }
    void takeDamage(int amount) override { enemy.stats.data.hitpoints -= amount; }
    const std::vector<string>& getDebuffs() const override { return enemy.debuffs; }
    void applyDebuff(const string& debuff) override { enemy.debuffs.push_back(debuff); }

private:
    Enemy& enemy;
};

class CombatSystem {
public:
    CombatResult attack(ICombatant& attacker, ICombatant& target) {
        static std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        
        bool dodge = false;
        if (dist(gen) < target.getDodgeRate()) {
            dodge = true;
        }

        int physicalDamage = 0;
        int magicalDamage = 0;
        int totalDamage = 0;
        bool isCrit = false;
        string debuffInflicted = "";

        if (!dodge) {
            physicalDamage = calculateDamage(attacker.getPhysicalAttack(), target.getArmor());
            magicalDamage = calculateDamage(attacker.getMagicAttack(), target.getMagicArmor());
            totalDamage = physicalDamage + magicalDamage;

         
            if (dist(gen) < attacker.getCritRate()) {
                isCrit = true;
                totalDamage = static_cast<int>(std::round(totalDamage * attacker.getCritDamage()));
            }

            target.takeDamage(totalDamage);

            
            if (attacker.isPlayer()) {
                PlayerCombatant* playerAttacker = dynamic_cast<PlayerCombatant*>(&attacker);
                if (playerAttacker) {
                    const auto& weaponDebuffs = playerAttacker->getEquippedWeaponDebuffs();
                    float chance = playerAttacker->getEquippedWeaponDebuffChance();
                    if (!weaponDebuffs.empty() && dist(gen) < chance) {
                        
                        std::uniform_int_distribution<size_t> debuffDist(0, weaponDebuffs.size() - 1);
                        debuffInflicted = weaponDebuffs[debuffDist(gen)];
                        target.applyDebuff(debuffInflicted);
                    }
                }
            }
        }

        return {attacker.getName(), target.getName(), physicalDamage, magicalDamage, totalDamage, isCrit, dodge, debuffInflicted};
    }

private:
    int calculateDamage(int attack, float defense) {
        float baseDamage = static_cast<float>(attack) * (1.0f - defense);
        static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<float> dist(-baseDamage * 0.15f, baseDamage * 0.15f);
        float damage = baseDamage + dist(gen);
        return static_cast<int>(std::round(std::max<float>(damage, 0.0f)));
    }
};

class Tavern {
public:
    Tavern(PlayerInventory& inv, std::vector<NPC>& party) : inventory(inv), playerParty(party) {}

    void openTavern(Player& player, TimeSystem& timeSystem) {
        bool inTavern = true;
        while (inTavern) {
            std::vector<string> lines = {
                "1. Sleep for 1 Gold (Restore HP & Advance Time)",
                "2. Buy Food and Drinks",
                "3. Hire a Party Member (Max 4)",
                "4. Exit"
            };
            
            cout << "\n--- THE RATTLING FLAGON TAVERN ---" << endl;
            displayBorderedMenu(lines, "The Tavernkeeper wipes a mug. 'What'll it be?' ");
            
            int choice = getNumberInput(1, 4);

            if (choice == 1) {
                if (player.economy.subtractCurrency(0, 1, 0, 0)) {
                    player.stats.hitpoints = player.stats.maxHitpoints;
                    player.sleptToday = true;
                    timeSystem.advanceTime(player);
                    cout << "\nYou settle into a hay-stuffed mattress. The world fades to black...\n";
                    cout << "You wake up feeling refreshed! (HP Restored)\n";
                } else {
                    cout << "\n'No coin, no bed,' the Tavernkeeper grunts.\n";
                }
            } 
            else if (choice == 2) { 
                buyFoodAndDrinks(player);
            } 
            else if (choice == 3) {
                hirePartyMember(player);
            } 
            else if (choice == 4) {
                cout << "You step back out into the cold air.\n";
                inTavern = false;
            }
        }
    }

private:
    PlayerInventory& inventory;
    std::vector<NPC>& playerParty;
    FoodandDrinksDatabase foodDB;
    NPCGenerator npcGen;

    void buyFoodAndDrinks(Player& player) {
        const auto& foods = foodDB.getFoodAndDrink();
        std::vector<string> foodNames;
        for (const auto& food : foods) {
            
            string price = std::to_string(food.priceGold) + "g " + std::to_string(food.priceSilver) + "s";
            foodNames.push_back(food.name + " (" + price + ")");
        }

        cout << "\n--- Available ---" << endl;
        PagedSelector foodSelector(foodNames);
        int index = foodSelector.select();

        if (index == -1) return;

        if (player.economy.subtractCurrency(foods[index].pricePlatinum, foods[index].priceGold, foods[index].priceSilver, foods[index].priceCopper)) {
            inventory.addItem(PlayerInventory::ItemType::FoodAndDrink, index);
            cout << "You purchased " << foods[index].name << ". It smells... edible.\n";
        } else {
            cout << "You don't have enough coin for that.\n";
        }
    }

    void hirePartyMember(Player& player) {
        if (playerParty.size() >= 4) {
            cout << "\n'Your group is too big already,' the Tavernkeeper remarks. (Max 4 members)\n";
            return;
        }

        NPC newNPC = npcGen.generateNPC(player.stats.level);

        
        int baseCost = 10;
        int levelMultiplier = newNPC.level * 2;
        int statBonus = (newNPC.stats.attack + newNPC.stats.magicAttack + newNPC.stats.armor + newNPC.stats.magicArmor) / 10;
        int totalCost = baseCost + levelMultiplier + statBonus;

        cout << "\nIn the corner, you see " << newNPC.name << ", a " << newNPC.race.name << " " << newNPC.playerClass.name << " (Lv " << newNPC.level << ").\n";
        cout << "Hiring cost: " << totalCost << " Gold.\n";
        cout << "Gear: " << (newNPC.equippedWeapon.empty() ? "Rags" : newNPC.equippedWeapon) << "\n";
        
        cout << "Confirm hire? (y/n): ";
        char confirm;
        cin >> confirm;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (confirm != 'y' && confirm != 'Y') {
            cout << "You decide not to hire them.\n";
            return;
        }

        if (!player.economy.subtractCurrency(0, totalCost, 0, 0)) {
            cout << "You realize you can't afford their services.\n";
            return;
        }

        newNPC.wagePerWeek = totalCost / 10;
        playerParty.push_back(newNPC);

        cout << "\n" << newNPC.name << " stands up and joins your cause!\n";

        if (!newNPC.dialogues.empty()) {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<size_t> dist(0, newNPC.dialogues.size() - 1);
            cout << newNPC.name << " says: \"" << newNPC.dialogues[dist(gen)] << "\"\n" << endl;
        }

        
        std::vector<string> specialNames = {"Evelyn Chevalier", "Astra Yao", "Ye Shunguang ", "Burnice White","Jane Doe", "Belle"};
        if (std::find(specialNames.begin(), specialNames.end(), newNPC.name) != specialNames.end()) {
            player.hiredSpecialCharacters.insert(newNPC.name);
            player.hasNewDictionaryEntry = true;
        }
    }
};

class Store {
public:
    Store(PlayerInventory& inv) : inventory(inv) {}

    void openStore(Player& player) {
        bool shopping = true;
        while (shopping) {
            cout << "\n=== STORE ===\n";
            cout << "Currency: " << player.economy.platinum << "p " << player.economy.gold << "g " << player.economy.silver << "s " << player.economy.copper << "c" << endl;
            cout << "1. Buy Potions\n";
            cout << "2. Buy Equipment\n";
            cout << "3. Exit Store\n";
            cout << "Choose an option: ";

            int choice = getNumberInput(1, 3);

            switch (choice) {
                case 1:
                    buyPotions(player);
                    system("cls");
                    break;
                case 2:
                    buyEquipment(player);
                    system("cls");
                    break;
                case 3:
                    shopping = false;
                    break;
            }
        }
    }

private:
    PlayerInventory& inventory;
    PotionDatabase potionDB;
    EquipmentandWeaponDatabase equipmentDB;

    void buyPotions(Player& player) {
        const auto& potions = potionDB.getPotions();
        std::vector<string> potionNames;
        for (const auto& potion : potions) {
            potionNames.push_back(potion.name + " - " + std::to_string(potion.priceSilver) + "s " + std::to_string(potion.priceCopper) + "c");
        }
        PagedSelector potionSelector(potionNames);
        size_t index = potionSelector.select();
        if (player.economy.subtractCurrency(0, 0, potions[index].priceSilver, potions[index].priceCopper)) {
            inventory.addItem(PlayerInventory::ItemType::Potion, index);
            cout << "Bought " << potions[index].name << "!\n";
        } else {
            cout << "Not enough currency!\n";
        }
    }

    void buyEquipment(Player& player) {
        const auto& equipment = equipmentDB.getEquipment();
        std::vector<string> equipmentNames;
        for (const auto& eq : equipment) {
            equipmentNames.push_back(eq.name + " - " + std::to_string(eq.priceSilver) + "s " + std::to_string(eq.priceCopper) + "c");
        }
        PagedSelector equipmentSelector(equipmentNames);
        size_t index = equipmentSelector.select();
        if (player.economy.subtractCurrency(0, 0, equipment[index].priceSilver, equipment[index].priceCopper)) {
            inventory.addItem(PlayerInventory::ItemType::Equipment, index);
            cout << "Bought " << equipment[index].name << "!\n";
            if (equipment[index].type == "Weapon") {
                player.boughtWeapons.insert(equipment[index].name);
                player.hasNewDictionaryEntry = true;
            }
        } else {
            cout << "Not enough currency!\n";
        }
    }
};
class magicStore {
public:
    magicStore(PlayerInventory& inv) : inventory(inv), debuffDB() {}

    void openStore(Player& player) {
        bool shopping = true;
        while (shopping) {
            cout << "\n=== MAGIC STORE ===\n";
            cout << "Currency: " << player.economy.platinum << "p " << player.economy.gold << "g " << player.economy.silver << "s " << player.economy.copper << "c" << endl;
            cout << "1. Buy Spells\n";
            cout << "2. Enchant Item\n";
            cout << "3. Exit Store\n";
            cout << "Choose an option: ";

            int choice = getNumberInput(1, 3);

            switch (choice) {
                case 1:
                    buySpells(player);
                    system("cls");
                    break;
                case 2:
                    enchantItem(player);
                    system("cls");
                    break;
                case 3:
                    shopping = false;
                    break;
            }
        }
    }

private:
    PlayerInventory& inventory;
    SpellDatabase spellDB;
    DebuffDatabase debuffDB;

    void buySpells(Player& player) {
        const auto& spells = spellDB.getSpells();
        std::vector<size_t> availableSpells;
        for (size_t i = 0; i < spells.size(); ++i) {
            if (spells[i].requiredLevel <= player.stats.level) {
                availableSpells.push_back(i);
            }
        }

        if (availableSpells.empty()) {
            cout << "No spells available for your level.\n";
            return;
        }

        std::vector<string> spellNames;
        for (size_t i : availableSpells) {
            const auto& spell = spells[i];
            spellNames.push_back(spell.spellName + " - " + std::to_string(spell.pricePlatinum) + "p " + std::to_string(spell.priceGold) + "g " + std::to_string(spell.priceSilver) + "s " + std::to_string(spell.priceCopper) + "c");
        }
        PagedSelector spellSelector(spellNames);
        size_t selectedIndex = spellSelector.select();
        size_t index = availableSpells[selectedIndex];
        const auto& spell = spells[index];
        if (player.economy.subtractCurrency(spell.pricePlatinum, spell.priceGold, spell.priceSilver, spell.priceCopper)) {
            
            if (std::find(player.learnedSpells.begin(), player.learnedSpells.end(), spell.spellName) == player.learnedSpells.end()) {
                player.learnedSpells.push_back(spell.spellName);
                cout << "Learned " << spell.spellName << "!\n";
            } else {
                cout << "You already know " << spell.spellName << "!\n";
            }
        } else {
            cout << "Not enough currency!\n";
        }
    }

    void enchantItem(Player& player) {
        
        std::vector<size_t> enchantableIndices;
        const auto& equipment = inventory.equipmentDB.getEquipment();
        for (size_t i = 0; i < equipment.size(); ++i) {
            if (equipment[i].type == "Weapon" || equipment[i].type == "Staff") {
               
                bool isEquipped = (equipment[i].type == "Weapon" && inventory.equipped.weaponIndex == static_cast<int>(i)) ||
                                  (equipment[i].type == "Staff" && inventory.equipped.staffIndex == static_cast<int>(i));
                if (isEquipped) {
                    enchantableIndices.push_back(i);
                }
            }
        }

        if (enchantableIndices.empty()) {
            cout << "No equipped enchantable items.\n";
            return;
        }

        cout << "\n=== ENCHANTABLE ITEMS ===\n";
        for (size_t j = 0; j < enchantableIndices.size(); ++j) {
            size_t i = enchantableIndices[j];
            cout << j + 1 << ". " << equipment[i].name << " (" << equipment[i].type << ")\n";
        }
        cout << "Choose item to enchant (0 to cancel): ";
        int itemChoice = getNumberInput(0, static_cast<int>(enchantableIndices.size()));
        if (itemChoice == 0) return;

        size_t itemIndex = enchantableIndices[static_cast<size_t>(itemChoice - 1)];
        const auto& item = equipment[itemIndex];

    
        if (!player.economy.subtractCurrency(0, 100, 0, 0)) {
            cout << "Not enough gold! Enchanting costs 100 gold.\n";
            return;
        }

        if (item.type == "Weapon") {
           
            const auto& availableDebuffs = debuffDB.getDebuffs();
            std::vector<string> debuffNames;
            for (const auto& debuff : availableDebuffs) {
                debuffNames.push_back(debuff.name + " - " + debuff.effectDesc);
            }
            PagedSelector debuffSelector(debuffNames);
            size_t debuffIndex = debuffSelector.select();
            const auto& debuff = availableDebuffs[debuffIndex];

           
            player.equippedWeaponDebuffs.push_back(debuff.name);
            player.equippedWeaponDebuffChance = item.debuffChance; 

  
            for (auto& invItem : inventory.inventory) {
                if (invItem.type == PlayerInventory::ItemType::Equipment && invItem.dbIndex == itemIndex) {
                    invItem.enchanted = true;
                }
            }

            cout << "Enchanted " << item.name << " with " << debuff.name << "!\n";
        } else if (item.type == "Staff") {
          
            const auto& spells = spellDB.getSpells();
            std::vector<string> spellNames;
            for (const auto& spell : spells) {
                spellNames.push_back(spell.spellName + " - " + spell.description);
            }
            PagedSelector spellSelector(spellNames);
            size_t spellIndex = spellSelector.select();
            const auto& spell = spells[spellIndex];

           
            player.equippedStaffSpells.push_back(spell.spellName);

      
            player.stats.magicAttack += static_cast<int>(std::round(spell.healthDamage * spell.staffBonusModifier));

            cout << "Enchanted " << item.name << " with " << spell.spellName << "!\n";
        }
    }
};

class CombatScreen {
public:
    CombatScreen(Player& pl, std::vector<NPC>& prty, Enemy& en, TimeSystem& ts, NPCGenerator& gen, SpellDatabase& sdb)
        : player(pl), party(prty), enemy(en), playerC(pl), enemyC(en), attackInfos(), timeSystem(ts), npcGen(gen), spellDB(sdb) {}

    void startCombat(CombatSystem& combat, PlayerInventory& inventory) {
        bool inCombat = true;

        while (inCombat && (player.stats.hitpoints > 0 || !party.empty()) && enemy.stats.data.hitpoints > 0) {
            displayCombatScreen();

         
            int action = getPlayerActionInput();
            switch (action) {
                case 1: 
                    handlePlayerAttack(combat);
                    break;
                case 2: 
                    inventory.showInventory(player);
                    break;
                case 3:
                    cout << "You ran away!" << endl;
                    inCombat = false;
                    continue;
                case 4:
                    handleCastSpell();
                    break;
                default:
                    cout << "Invalid choice!" << endl;
                    break;
            }

           
            for (auto it = party.begin(); it != party.end(); ) {
                if (it->stats.hitpoints > 0) {
                    NPCCombatant npcC(*it);
                    CombatResult result = combat.attack(npcC, enemyC);
                    string info = result.attackerName + " attacked " + result.targetName;
                    if (result.dodge) {
                        info += " but " + result.targetName + " dodged!";
                    } else {
                        info += " and dealt " + std::to_string(result.totalDamage) + " damage" + (result.isCrit ? " (Critical Hit!)" : "") + ".";
                    }
                    attackInfos.push_back(info);
                    ++it;
                } else {
              
                    npcGen.unlockName(it->name);
                    it = party.erase(it);
                }
            }

         
            handleEnemyTurn(combat);

            inventory.tickBuffs(player);
            system("cls");

            for (auto it = party.begin(); it != party.end(); ) {
                if (it->stats.hitpoints <= 0) {
                    npcGen.unlockName(it->name);
                    it = party.erase(it);
                } else {
                    ++it;
                }
            }
        }

        displayCombatOutcome();
    }

private:
    Player& player;
    std::vector<NPC>& party;
    Enemy& enemy;
    PlayerCombatant playerC;
    EnemyCombatant enemyC;
    std::vector<string> attackInfos;
    TimeSystem timeSystem;
    NPCGenerator& npcGen;
    SpellDatabase& spellDB;



    void displayCombatScreen() {
        std::vector<string> lines;
        lines.push_back(player.name + " - HP: " + std::to_string(player.stats.hitpoints) + "/" + std::to_string(player.stats.maxHitpoints) + " MP: " + std::to_string(player.stats.mana) + "/" + std::to_string(player.stats.maxMana));
        if (!player.debuffs.empty()) {
            string debuffs = "Debuffs: ";
            for (const auto& debuff : player.debuffs) {
                debuffs += debuff + " ";
            }
            lines.push_back(debuffs);
        }
        for (const auto& npc : party) {
            lines.push_back(npc.name + " - HP: " + std::to_string(npc.stats.hitpoints) + "/" + std::to_string(npc.stats.maxHitpoints));
            if (!npc.debuffs.empty()) {
                string debuffs = "Debuffs: ";
                for (const auto& debuff : npc.debuffs) {
                    debuffs += debuff + " ";
                }
                lines.push_back(debuffs);
            }
        }
        lines.push_back(enemy.name + " - HP: " + std::to_string(enemy.stats.data.hitpoints) + "/" + std::to_string(enemy.stats.data.maxHitpoints));
        if (!enemy.debuffs.empty()) {
            string debuffs = "Debuffs: ";
            for (const auto& debuff : enemy.debuffs) {
                debuffs += debuff + " ";
            }
            lines.push_back(debuffs);
        }
        for (const auto& info : attackInfos) {
            lines.push_back(info);
        }
        displayBorderedMenu(lines, "");
        attackInfos.clear();
    }

    int getPlayerActionInput() const {
        cout << "\nChoose your action:" << endl;
        cout << "1. Attack" << endl;
        cout << "2. Use Item / Potion" << endl;
        cout << "3. Run" << endl;
        if (!player.learnedSpells.empty()) {
            cout << "4. Cast Spell" << endl;
            return getNumberInput(1, 4);
        }

        return getNumberInput(1, 3);
    }

    void handlePlayerAttack(CombatSystem& combat) {
        CombatResult result = combat.attack(playerC, enemyC);
        string info = result.attackerName + " attacked " + result.targetName;
        if (result.dodge) {
            info += " but " + result.targetName + " dodged!";
        } else {
            info += " and dealt " + std::to_string(result.totalDamage) + " damage" + (result.isCrit ? " (Critical Hit!)" : "") + ".";
        }
        attackInfos.push_back(info);
    }

    void handleCastSpell() {
        if (player.learnedSpells.empty()) return;

        cout << "Choose a spell to cast:" << endl;
        for (size_t i = 0; i < player.learnedSpells.size(); ++i) {
            cout << i + 1 << ". " << player.learnedSpells[i] << endl;
        }
        int choice = getNumberInput(1, static_cast<int>(player.learnedSpells.size()));
        string spellName = player.learnedSpells[choice - 1];

        const auto& spells = spellDB.getSpells();
        auto it = std::find_if(spells.begin(), spells.end(), [&](const SpellDatabase::SpellData& s){ return s.spellName == spellName; });
        if (it == spells.end()) return;
        const auto& spell = *it;

        if (player.stats.mana < spell.manaCost) {
            cout << "Mana insufficient" << endl;
            return;
        }

        player.stats.mana -= spell.manaCost;
        string info = player.name + " cast " + spell.spellName;

        if (spell.healthDamage > 0) {
            enemy.stats.data.hitpoints -= spell.healthDamage;
            info += " and dealt " + std::to_string(spell.healthDamage) + " damage";
        }
        if (spell.manaDamage > 0) {
            enemy.stats.data.mana -= spell.manaDamage;
            if (enemy.stats.data.mana < 0) enemy.stats.data.mana = 0;
            info += " and drained " + std::to_string(spell.manaDamage) + " mana";
        }
        if (spell.healthRestore > 0) {
            int heal = std::min(spell.healthRestore, player.stats.maxHitpoints - player.stats.hitpoints);
            player.stats.hitpoints += heal;
            info += " and healed " + std::to_string(heal) + " HP";
        }
        if (spell.manaRestore > 0) {
            int restore = std::min(spell.manaRestore, player.stats.maxMana - player.stats.mana);
            player.stats.mana += restore;
            info += " and restored " + std::to_string(restore) + " mana";
        }
        if (spell.armorIncrease > 0) {
            player.stats.armor += spell.armorIncrease;
            info += " and increased armor";
        }
        if (spell.magicArmorIncrease > 0) {
            player.stats.magicArmor += spell.magicArmorIncrease;
            info += " and increased magic armor";
        }
        if (spell.hasDebuff) {
            for (const auto& debuff : spell.debuffs) {
                enemy.debuffs.push_back(debuff);
                info += " and applied " + debuff;
            }
        }

        attackInfos.push_back(info + ".");
    }

    void handleEnemyTurn(CombatSystem& combat) {
        if (enemy.stats.data.hitpoints <= 0 || player.stats.hitpoints <= 0)
            return;
        CombatResult result = combat.attack(enemyC, playerC);
        string info = result.attackerName + " attacked " + result.targetName;
        if (result.dodge) {
            info += " but " + result.targetName + " dodged!";
        } else {
            info += " and dealt " + std::to_string(result.totalDamage) + " damage" + (result.isCrit ? " (Critical Hit!)" : "") + ".";
        }
        attackInfos.push_back(info);
    }

    void displayCombatOutcome() const {
        if (player.stats.hitpoints <= 0)
            cout << player.name << " has been defeated!" << endl;
        else if (enemy.stats.data.hitpoints <= 0)
            cout << enemy.name << " has been defeated!" << endl;

    }
};

void handleEvent(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, NPCGenerator& npcGen) {
    eventDatabase eventDB;
    SpellDatabase spellDB;
    const auto& events = eventDB.getEvents();
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, events.size() - 1);
    const auto& event = events[dist(gen)];

    cout << "\n=== EVENT: " << event.name << " ===" << endl;
    cout << event.description << endl;


    if (event.goldReward != 0) {
        if (event.goldReward > 0) {
            hero.economy.addCurrency(0, 0, event.goldReward, 0);
            cout << "You gained " << event.goldReward << " silver!" << endl;
        } else {
            int goldToSubtract = -event.goldReward;
            if (hero.economy.subtractCurrency(0, 0, goldToSubtract, 0)) {
                cout << "You lost " << goldToSubtract << " silver!" << endl;
            } else {
                cout << "You didn't have enough gold to lose!" << endl;
            }
        }
    }

    if (event.expReward != 0) {
        if (event.expReward > 0) {
            hero.stats.expe += event.expReward;
            cout << "You gained " << event.expReward << " experience!" << endl;
            heroStats.levelUpChecker();
        } else {
            int expToSubtract = -event.expReward;
            hero.stats.expe = std::max<float>(0.0f, hero.stats.expe - static_cast<float>(expToSubtract));
            cout << "You lost " << expToSubtract << " experience!" << endl;
        }
    }

    if (event.healthEffect != 0) {
        if (event.healthEffect > 0) {
            int heal = std::min(event.healthEffect, hero.stats.maxHitpoints - hero.stats.hitpoints);
            hero.stats.hitpoints += heal;
            cout << "You recovered " << heal << " HP!" << endl;
        } else {
            int damage = -event.healthEffect;
            hero.stats.hitpoints -= damage;
            if (hero.stats.hitpoints <= 0) {
                hero.stats.hitpoints = 1;
                cout << "You took " << damage << " damage but survived!" << endl;
            } else {
                cout << "You took " << damage << " damage!" << endl;
            }
        }
    }

    if (event.manaEffect != 0) {
        if (event.manaEffect > 0) {
            int restore = std::min(event.manaEffect, hero.stats.maxMana - hero.stats.mana);
            hero.stats.mana += restore;
            cout << "You recovered " << restore << " Mana!" << endl;
        } else {
            int drain = -event.manaEffect;
            hero.stats.mana = std::max<int>(0, hero.stats.mana - drain);
            cout << "You lost " << drain << " Mana!" << endl;
        }
    }


    if (event.enemyEncounter && !event.enemyName.empty()) {
        cout << "\nYou encounter an enemy!" << endl;
        Enemy enemy = enemyCtrl.getEnemyByName(event.enemyName, hero.stats.level, hero.currentLocationType);
        std::vector<NPC> emptyParty;
        system("cls");
        CombatScreen combatScreen(hero, emptyParty, enemy, hero.timeSystem, npcGen, spellDB);
        combatScreen.startCombat(combat, playerInventory);
        system("cls");

        if (hero.stats.hitpoints > 0 && enemy.stats.data.hitpoints <= 0) {
            enemyCtrl.enemyGoldExpDrop(hero, enemy);
            heroStats.levelUpChecker();
        }
    }

   
    if (hero.encounteredEvents.find(event.name) == hero.encounteredEvents.end()) {
        hero.encounteredEvents.insert(event.name);
        hero.hasNewDictionaryEntry = true;
    }

    cout << "Press Enter to continue...";
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

class TravelSystem{
    public:
        TravelSystem(NPCGenerator& gen, bool debugAllDiscovered = false) : npcGen(gen), discovered(locationDB.getLocations().size(), debugAllDiscovered), marked(locationDB.getLocations().size(), false) {}

        void travel(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem) {
        system("cls");
        std::vector<string> lines = {
            "1. Venture into the unknown -- uncover new lands and dangers",
            "2. Follow safe routes -- travel to known, marked locations"
        };
            displayBorderedMenu(lines, "Choose an option: ");
            int choice = getNumberInput(1, 2);

    if (choice == 1) {
        system("cls");
        exploreRandomLocation(hero, enemyCtrl, combat, playerInventory, heroStats, timeSystem);
    } else {
        system("cls");
        const auto& locations = locationDB.getLocations();
                std::vector<size_t> markedLocations;
                for (size_t i = 0; i < locations.size(); ++i) {
                    if (marked[i]) {
                        markedLocations.push_back(i);
                    }
                }
                if (markedLocations.empty()) {
                    cout << "No marked locations.\n";
                    return;
                }
                for (size_t j = 0; j < markedLocations.size(); ++j) {
                    size_t i = markedLocations[j];
                    cout << j + 1 << ". " << locations[i].name << endl;
                }
                cout << "Choose a location: ";
                int locChoice = getNumberInput(1, static_cast<int>(markedLocations.size()));
                size_t idx = markedLocations[locChoice - 1];
                hero.currentLocation = locations[idx].name;
                hero.currentLocationType = locations[idx].type;
                enterLocation(hero, enemyCtrl, combat, playerInventory, heroStats, timeSystem, idx, true);
            }
        }

        bool hasDiscoveredLocations() const {
            return std::any_of(discovered.begin(), discovered.end(), [](bool d){ return d; });
        }

        void exploreRandomLocation(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem) {
            const auto& locations = locationDB.getLocations();
            static std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<size_t> dist(0, locations.size() - 1);
            size_t idx = dist(gen);
            bool firstTime = !discovered[idx];
            discovered[idx] = true;
    if (firstTime) {
        hero.stats.expe += 50.0f;
        cout << "\nDiscovered new location: " << locations[idx].name << "! Gained 50 experience.\n";
        heroStats.levelUpChecker();
        hero.discoveredLocations.insert(locations[idx].name);
        hero.hasNewDictionaryEntry = true;
        
    }
    hero.currentLocation = locations[idx].name;
    hero.currentLocationType = locations[idx].type;
    enterLocation(hero, enemyCtrl, combat, playerInventory, heroStats, timeSystem, idx);
        }

    private:
        NPCGenerator& npcGen;
        locationDatabase locationDB;
        std::vector<bool> discovered;
        std::vector<bool> marked;

    void enterLocation(Player& hero, EnemyController& enemyCtrl, CombatSystem& combat, PlayerInventory& playerInventory, PlayerController& heroStats, TimeSystem& timeSystem, size_t locationIndex, bool isSafe = false) {
        const auto& location = locationDB.getLocations()[locationIndex];
        SpellDatabase spellDB;
        bool inLocation = true;
        while (inLocation) {
        system("cls");

            cout << "\n=== " << location.name << " ===\n";
            cout << location.description << endl;
            cout << "1. Explore\n";
            cout << "2. Mark location\n";
            cout << "3. Leave\n";
            cout << "Choose an action: ";
            int action = getNumberInput(1, 3);

            switch (action) {
                case 1: {
                    float enemyChance = isSafe ? 0.0f : 0.5f;
                    if (!isSafe) {
                        switch (location.type) {
                            case PeacefulVillage: enemyChance = 0.2f; break;
                            case PeacefulTown: enemyChance = 0.3f; break;
                            case Dungeon: enemyChance = 0.8f; break;
                            case Terrain: enemyChance = 0.6f; break;
                            default: enemyChance = 0.5f; break;
                        }
                    }

                    static std::mt19937 gen(std::random_device{}());
                    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                if (dist(gen) < enemyChance) {
                   
                    Enemy enemy = enemyCtrl.encounterEnemy(location.difficultyLevel, location.type);
                    std::vector<NPC> emptyParty;
                    NPCGenerator localNpcGen;
                    CombatScreen combatScreen(hero, emptyParty, enemy, timeSystem, localNpcGen, spellDB);
                    combatScreen.startCombat(combat, playerInventory);

                    if (hero.stats.hitpoints > 0 && enemy.stats.data.hitpoints <= 0) {
                        enemyCtrl.enemyGoldExpDrop(hero, enemy);
                        heroStats.levelUpChecker();
                    }
                }
  
                handleEvent(hero, enemyCtrl, combat, playerInventory, heroStats, npcGen);
                timeSystem.advanceTime(hero);
                break;
                }
                case 2:
                    marked[locationIndex] = true;
                    cout << "Location marked for safe travel.\n";
                    break;
                case 3:
                    inLocation = false;
                    break;
            }
        }
    }
};

struct MenuItem {
    string name;
    string description;
    std::function<void()> action;
};
class Menu {
public:
    Menu(const std::vector<MenuItem>& menuItems, const std::map<string, std::vector<size_t>>& menuCategories, Player& heroRef)
        : items(menuItems), categories(menuCategories), hero(heroRef) {}

    void displayAndExecute() {
        while (true) {
            display();
            int choice = getNumberInput(1, static_cast<int>(items.size()));
            if (choice >= 1 && choice <= static_cast<int>(items.size())) {
                items[static_cast<size_t>(choice - 1)].action();
                break;
            } else {
                cout << "Invalid choice!" << endl;
            }
        }
    }

private:
    std::vector<MenuItem> items;
    std::map<string, std::vector<size_t>> categories;
    Player& hero;

    void display() {
  
        size_t maxWidth = 0; 

        string timeLine = "Current Time: " + hero.timeSystem.getPeriodString();
        string passedLine = "Time Passed: " + hero.timeSystem.getFormattedTimePassed();
        string locationLine = "Current Location: " + hero.currentLocation;
        string currencyLine = "Currency: " + std::to_string(hero.economy.platinum) + "p " +
                             std::to_string(hero.economy.gold) + "g " +
                             std::to_string(hero.economy.silver) + "s " +
                             std::to_string(hero.economy.copper) + "c";

        maxWidth = std::max(maxWidth, timeLine.length());
        maxWidth = std::max(maxWidth, passedLine.length());
        maxWidth = std::max(maxWidth, locationLine.length());
        maxWidth = std::max(maxWidth, currencyLine.length());

       
        for (const auto& cat : categories) {
            if (!cat.second.empty()) {
                string catLine = "[" + cat.first + "]";
                maxWidth = std::max(maxWidth, catLine.length());
            }
        }

        
        for (const auto& item : items) {
            string optionLine = std::to_string(&item - &items[0] + 1) + ". " + item.name + " - " + item.description;
            maxWidth = std::max(maxWidth, optionLine.length());
        }

        string border = string(maxWidth, '=');
        string emptyLine = string(maxWidth, ' ');

        cout << border << endl;
        cout << timeLine << string(maxWidth - timeLine.length(), ' ') << endl;
        cout << passedLine << string(maxWidth - passedLine.length(), ' ') << endl;
        cout << locationLine << string(maxWidth - locationLine.length(), ' ') << endl;
        cout << currencyLine << string(maxWidth - currencyLine.length(), ' ') << endl;
        cout << emptyLine << endl;

       
        std::vector<string> categoryOrder = {"Character", "Local Establishments", "Actions", "System"};

        for (const string& catName : categoryOrder) {
            auto it = categories.find(catName);
            if (it != categories.end() && !it->second.empty()) {
                string catLine = "[" + catName + "]";
                cout << catLine << string(maxWidth - catLine.length(), ' ') << endl;
                for (size_t idx : it->second) {
                    string optionLine = std::to_string(idx + 1) + ". " + items[idx].name + " - " + items[idx].description;
                    cout << optionLine << string(maxWidth - optionLine.length(), ' ') << endl;
                }
                cout << emptyLine << endl;
            }
        }

        cout << border << endl;
        cout << "Choose an option: ";
    }
};

void deductWeeklyWages(Player& player, std::vector<NPC>& playerParty) {
    int totalWages = 0;
    for (const auto& npc : playerParty) {
        totalWages += npc.wagePerWeek;
    }
    if (player.economy.subtractCurrency(0, totalWages, 0, 0)) {
        cout << "Paid " << totalWages << " gold in wages to party members.\n";
    } else {
        cout << "Not enough gold to pay wages! Party members may become unhappy.\n";
    }
}

void manageParty(std::vector<NPC>& playerParty, NPCGenerator& npcGen, Player& player) {
    if (playerParty.empty()) {
        cout << "Your party is empty.\n";
        cout << "Press Enter to continue...";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    
    int totalWages = 0;
    for (const auto& npc : playerParty) {
        totalWages += npc.wagePerWeek;
    }
    if (!player.economy.subtractCurrency(0, totalWages, 0, 0)) {
       
        static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, playerParty.size() - 1);
        size_t index = dist(gen);
        string name = playerParty[index].name;
        npcGen.unlockName(name);
        playerParty.erase(playerParty.begin() + static_cast<int>(index));
        cout << "Due to insufficient gold, " << name << " has left the party.\n";
        cout << "Press Enter to continue...";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    } else {
        
        player.economy.addCurrency(0, totalWages, 0, 0);
    }

    std::vector<string> lines;
    lines.push_back("Current Party Members:");
    for (size_t i = 0; i < playerParty.size(); ++i) {
        const auto& npc = playerParty[i];
        string line = std::to_string(i + 1) + ". " + npc.name + " (" + npc.race.name + " " + npc.playerClass.name + ", Level " + std::to_string(npc.level) + ") - Weekly Wage: " + std::to_string(npc.wagePerWeek) + " gold";
        lines.push_back(line);
    }
    lines.push_back("Select a member (0 to cancel):");
    displayBorderedMenu(lines, "");

    int choice = getNumberInput(0, static_cast<int>(playerParty.size()));
    if (choice == 0) return;

    size_t index = static_cast<size_t>(choice - 1);

    cout << "What do you want to do with " << playerParty[index].name << "?" << endl;
    cout << "1. Talk to member" << endl;
    cout << "2. What is your story?" << endl;
    cout << "3. Kick member" << endl;
    cout << "4. Cancel" << endl;
    int action = getNumberInput(1, 4);

    if (action == 1) {

        if (!playerParty[index].dialogues.empty()) {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<size_t> dist(0, playerParty[index].dialogues.size() - 1);
            cout << playerParty[index].name << " says: \"" << playerParty[index].dialogues[dist(gen)] << "\"" << endl;
        } else {
            cout << playerParty[index].name << " has nothing to say." << endl;
        }
        cout << "Press Enter to continue...";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (action == 2) {
        cout << playerParty[index].name << "'s story: " << playerParty[index].story << endl;
        cout << "Press Enter to continue...";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (action == 3) {

        string name = playerParty[index].name;
        npcGen.unlockName(name);
        playerParty.erase(playerParty.begin() + static_cast<int>(index));
        cout << name << " has been kicked from the party.\n";
        cout << "Press Enter to continue...";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else {
        // Cancel
    }
}

void showDictionary(Player& hero) {
    hero.hasNewDictionaryEntry = false;
    bool inDictionary = true;
    while (inDictionary) {
        system("cls");
        std::vector<string> sections = {"1. Enemies", "2. Weapons", "3. Locations", "4. Events", "5. Special Characters", "6. Exit"};
        displayBorderedMenu(sections, "Choose a section: ");
        int choice = getNumberInput(1, 6);
        if (choice == 6) {
            inDictionary = false;
            continue;
        }
        if (choice == 1) { 
            if (hero.defeatedEnemies.empty()) {
                cout << "No defeated enemies yet.\n";
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::vector<string> enemyNames(hero.defeatedEnemies.begin(), hero.defeatedEnemies.end());
            PagedSelector enemySelector(enemyNames);
            size_t idx = enemySelector.select();
            string enemyName = enemyNames[idx];
            EnemyDatabase enemyDB;
            for (const auto& tmpl : enemyDB.templates) {
                if (tmpl.name == enemyName) {
                    bool inEnemy = true;
                    while (inEnemy) {
                        system("cls");
                        cout << "=== " << enemyName << " ===\n";
                        cout << "1. Stats\n";
                        cout << "2. Description\n";
                        cout << "3. Exit\n";
                        cout << "Choose: ";
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            cout << "HP: " << tmpl.stats.data.hitpoints << "/" << tmpl.stats.data.maxHitpoints << "\n";
                            cout << "Attack: " << tmpl.stats.data.attack << "\n";
                            cout << "Magic Attack: " << tmpl.stats.data.magicAttack << "\n";
                            cout << "Armor: " << tmpl.stats.data.armor << "\n";
                            cout << "Magic Armor: " << tmpl.stats.data.magicArmor << "\n";
                            cout << "Level: " << tmpl.stats.data.level << "\n";
                            cout << "Experience: " << tmpl.stats.data.expe << "\n";
                            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else if (subChoice == 2) {
                            cout << tmpl.lore.desc << "\n";
                            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else {
                            inEnemy = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 2) { 
            if (hero.boughtWeapons.empty()) {
                cout << "No bought weapons yet.\n";
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::vector<string> weaponNames(hero.boughtWeapons.begin(), hero.boughtWeapons.end());
            PagedSelector weaponSelector(weaponNames);
            size_t idx = weaponSelector.select();
            string weaponName = weaponNames[idx];
            EquipmentandWeaponDatabase eqDB;
            for (const auto& eq : eqDB.getEquipment()) {
                if (eq.name == weaponName) {
                    bool inWeapon = true;
                    while (inWeapon) {
                        system("cls");
                        cout << "=== " << weaponName << " ===\n";
                        cout << "1. Stats\n";
                        cout << "2. Description\n";
                        cout << "3. Exit\n";
                        cout << "Choose: ";
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            cout << "Type: " << eq.type << "\n";
                            cout << "Attack Increase: " << eq.attackIncrease << "\n";
                            cout << "Magic Attack Increase: " << eq.magicAttackIncrease << "\n";
                            cout << "Crit Rate Increase: " << eq.critRateIncrease << "\n";
                            cout << "Crit Damage Increase: " << eq.critDamageIncrease << "\n";
                            cout << "Price: " << eq.pricePlatinum << "p " << eq.priceGold << "g " << eq.priceSilver << "s " << eq.priceCopper << "c\n";
                            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else if (subChoice == 2) {
                            cout << eq.effectDesc << "\n";
                            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else {
                            inWeapon = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 3) { 
            if (hero.discoveredLocations.empty()) {
                cout << "No discovered locations yet.\n";
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::vector<string> locationNames(hero.discoveredLocations.begin(), hero.discoveredLocations.end());
            PagedSelector locationSelector(locationNames);
            size_t idx = locationSelector.select();
            string locationName = locationNames[idx];
            locationDatabase locDB;
            for (const auto& loc : locDB.getLocations()) {
                if (loc.name == locationName) {
                    bool inLocation = true;
                    while (inLocation) {
                        system("cls");
                        cout << "=== " << locationName << " ===\n";
                        cout << "1. Stats\n";
                        cout << "2. Description\n";
                        cout << "3. Exit\n";
                        cout << "Choose: ";
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            cout << "Difficulty Level: " << loc.difficultyLevel << "\n";
                            string typeStr;
                            switch (loc.type) {
                                case PeacefulVillage: typeStr = "Peaceful Village"; break;
                                case PeacefulTown: typeStr = "Peaceful Town"; break;
                                case Dungeon: typeStr = "Dungeon"; break;
                                case Terrain: typeStr = "Terrain"; break;
                                case SpellStore: typeStr = "Spell Store"; break;
                            }
                            cout << "Type: " << typeStr << "\n";
                            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else if (subChoice == 2) {
                            cout << loc.description << "\n";
                            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else {
                            inLocation = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 4) { 
            if (hero.encounteredEvents.empty()) {
                cout << "No encountered events yet.\n";
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::vector<string> eventNames(hero.encounteredEvents.begin(), hero.encounteredEvents.end());
            PagedSelector eventSelector(eventNames);
            size_t idx = eventSelector.select();
            string eventName = eventNames[idx];
            eventDatabase eventDB;
            for (const auto& ev : eventDB.getEvents()) {
                if (ev.name == eventName) {
                    bool inEvent = true;
                    while (inEvent) {
                        system("cls");
                        cout << "=== " << eventName << " ===\n";
                        cout << "1. Stats\n";
                        cout << "2. Description\n";
                        cout << "3. Exit\n";
                        cout << "Choose: ";
                        int subChoice = getNumberInput(1, 3);
                        if (subChoice == 1) {
                            cout << "Gold Reward: " << ev.goldReward << "\n";
                            cout << "Exp Reward: " << ev.expReward << "\n";
                            cout << "Health Effect: " << ev.healthEffect << "\n";
                            cout << "Mana Effect: " << ev.manaEffect << "\n";
                            cout << "Enemy Encounter: " << (ev.enemyEncounter ? "Yes" : "No") << "\n";
                            if (ev.enemyEncounter) {
                                cout << "Enemy: " << ev.enemyName << "\n";
                                cout << "Count: " << ev.enemyCount << "\n";
                            }
                            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else if (subChoice == 2) {
                            cout << ev.description << "\n";
                            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        } else {
                            inEvent = false;
                        }
                    }
                    break;
                }
            }
        } else if (choice == 5) {
            if (hero.hiredSpecialCharacters.empty()) {
                cout << "No hired special characters yet.\n";
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            system("cls");
            cout << "=== Special Characters ===\n";
            for (const auto& name : hero.hiredSpecialCharacters) {
                cout << "- " << name << "\n";
            }
            cout << "\nPress Enter to continue...";
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

void mainMenu(Player& hero, bool debugMode = false) {
    EnemyController enemyCtrl;
    CombatSystem combat;
    PlayerInventory playerInventory;
    PlayerController heroStats(hero, playerInventory);
    Store store(playerInventory);
    std::vector<NPC> playerParty;
    Tavern tavern(playerInventory, playerParty);
    magicStore magicStore(playerInventory);
    NPCGenerator npcGen;
    SpellDatabase spellDB;
    TravelSystem travelSystem(npcGen, debugMode);

    int actionCounter = 0;
    int lastWeekPaid = 0;
    bool running = true;

    while (running) {
        system("cls");

        if (hero.timeSystem.getTotalWeeks() > lastWeekPaid && !playerParty.empty()) {
            deductWeeklyWages(hero, playerParty);
            lastWeekPaid = hero.timeSystem.getTotalWeeks();
        }
        std::vector<MenuItem> items;
        std::map<string, std::vector<size_t>> categories;

        
        categories["Character"] = {};
        items.push_back({"Show Player Stats", "View your character's current stats.", [&]() { heroStats.showStats(); }});
        categories["Character"].push_back(items.size() - 1);
        items.push_back({"Inventory", "Manage your items and equipment.", [&]() { playerInventory.showInventory(hero); }});
        categories["Character"].push_back(items.size() - 1);
        items.push_back({"Party Management", "View and manage your party members.", [&]() { manageParty(playerParty, npcGen, hero); }});
        categories["Character"].push_back(items.size() - 1);
        string dictionaryName = hero.hasNewDictionaryEntry ? "Dictionary [!]" : "Dictionary";
        items.push_back({dictionaryName, "Review discovered enemies, locations, weapons, events, and special characters.", [&]() { showDictionary(hero); }});
        categories["Character"].push_back(items.size() - 1);

        bool hasStore = hero.currentLocationType == PeacefulVillage || hero.currentLocationType == PeacefulTown;
        bool hasTavern = hero.currentLocationType == PeacefulTown;
        bool hasMagicStore = hero.currentLocationType == SpellStore;

        if (hasStore || hasTavern || hasMagicStore) {
            categories["Local Establishments"] = {};
            if (hasStore) {
                items.push_back({"Store", "Buy potions and equipment.", [&]() { store.openStore(hero); }});
                categories["Local Establishments"].push_back(items.size() - 1);
            }
            if (hasTavern) {
                items.push_back({"Tavern", "Rest, buy food, hire party members.", [&]() { tavern.openTavern(hero, hero.timeSystem); }});
                categories["Local Establishments"].push_back(items.size() - 1);
            }
            if (hasMagicStore) {
                items.push_back({"Magic Store", "Buy spells.", [&]() { magicStore.openStore(hero); }});
                categories["Local Establishments"].push_back(items.size() - 1);
            }
        }

        categories["Actions"] = {};
        items.push_back({"Explore", "Venture out and face challenges.", [&]() {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);

            if (dist(gen) < 0.8f) {
                Enemy enemy = enemyCtrl.encounterEnemy(1, Terrain);
                CombatScreen combatScreen(hero, playerParty, enemy, hero.timeSystem, npcGen, spellDB);
                combatScreen.startCombat(combat, playerInventory);

                if (hero.stats.hitpoints > 0 && enemy.stats.data.hitpoints <= 0) {
                    enemyCtrl.enemyGoldExpDrop(hero, enemy);
                    heroStats.levelUpChecker();
                }
            } else {
                handleEvent(hero, enemyCtrl, combat, playerInventory, heroStats, npcGen);
            }

            actionCounter++;
            if (actionCounter % 4 == 0) {
                hero.timeSystem.advanceTime(hero);
            }
        }});
        categories["Actions"].push_back(items.size() - 1);

        items.push_back({"Travel", "Move to different locations.", [&]() {
            travelSystem.travel(hero, enemyCtrl, combat, playerInventory, heroStats, hero.timeSystem);
        }});
        categories["Actions"].push_back(items.size() - 1);

        items.push_back({"Pass Time", "Advance time without action.", [&]() {
            actionCounter++;
            if (actionCounter % 4 == 0) {
                hero.timeSystem.advanceTime(hero);
                cout << "Time has passed.\n";
                system("cls");
            }
        }});
        categories["Actions"].push_back(items.size() - 1);

    
        categories["System"] = {};
        items.push_back({"Basics", "Explain the game mechanics.", [&]() {
            system("cls");
            cout << "\n=== GAME BASICS ===\n";
            cout << "Time System:\n";
            cout << "- 4 turns advance the state of the day (Morning -> Afternoon -> Evening -> Night).\n";
            cout << "- After Night, a new day begins, and weeks accumulate.\n\n";
            cout << "Currency System:\n";
            cout << "- 100 Copper = 1 Silver\n";
            cout << "- 100 Silver = 1 Gold\n";
            cout << "- 100 Gold = 1 Platinum\n\n";
            cout << "Sleep Mechanic:\n";
            cout << "- If not slept, reduce health by ~5% at the start of each new day.\n";
            cout << "- Sleep at the Tavern to restore HP and avoid the penalty.\n\n";
            cout << "Other Mechanics:\n";
            cout << "- Explore to fight enemies or encounter events.\n";
            cout << "- Travel to discover new locations.\n";
            cout << "- Manage your party, inventory, and stats.\n";
            cout << "- Visit stores, taverns, and magic shops in towns.\n\n";
            cout << "Press Enter to continue...";
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }});
        categories["System"].push_back(items.size() - 1);
        items.push_back({"Exit", "Quit the game.", [&]() { running = false; }});
        categories["System"].push_back(items.size() - 1);

        Menu menu(items, categories, hero);
        menu.displayAndExecute();
        system("cls");
    }
}


void narrate(const string& text, int delay = 40) {
    for (char c : text) {
        cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

int main() {
    PlayerRaceDatabase raceDb;
    PlayerClassCollection classDb;

    narrate("\nThe sun is a dying ember...");
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    narrate(" and so are we.\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    system("cls");

    narrate("\nOh...");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    narrate(" The Forgotten Land.\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate("\nA name I have not tasted in a long, long while.\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate("\nA land drenched in the salt of old wars...");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    narrate(" where the screams of the damned still hum beneath the soil.\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate("\nThey say when you were born, ");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    narrate("the wind carried a sigh of pity.");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate("\nYour spirit is no great flame,");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    narrate(" it is but a flickering candle, ");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    narrate("just like all the rest.\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate("\nNothing new...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate("\nNothing changed...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate("\nSimply the inevitable... ");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    narrate("delayed for one more night.\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    string debugInput;
    narrate("\nDo you truly possess the will to endure this rot? \n>> ");
    getline(cin, debugInput);
    bool debugMode = (debugInput == "Quick Start");

    if (debugMode) {
        const PlayerRaceTemplate& chosenRace = raceDb.templates[0];
        string name = "Gwensent";
        const PlayerClassTemplate& chosenClass = classDb.templates[0];
        PlayerRace playerRace{ chosenRace.name, { chosenRace.lore.description } };
        Player hero(name, playerRace, chosenClass);
        hero.applyRaceBonus(chosenRace.statBonus);

        cout << "Press Enter to step into the dark...";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        mainMenu(hero, true);
        return 0;
    }


    narrate("\n--- SELECT YOUR ANCESTRY ---\n"); 
    std::vector<string> raceNames;
    for (const auto& r : raceDb.templates) raceNames.push_back(r.name);

    PagedSelector raceSelector(raceNames);
    size_t raceIndex = raceSelector.select();
    const PlayerRaceTemplate& chosenRace = raceDb.templates[raceIndex];
    system("cls");
    narrate("\nYour origin was no grand event...");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");
    narrate("\nThe world did not rejoice.\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");
    narrate("\nIt simply watched in silence");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    narrate(" as your mother whispered your name to the falling ash...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    string name;
    narrate("\nWhat name did she whisper? : ");
    getline(cin, name);
    system("cls");

    narrate("\nTime is cruel.");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    narrate(" Among the " + chosenRace.name);
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    narrate(", you learned that to love..... ");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    narrate("is to eventually mourn.\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate(" You sharpened your resolve...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");
    
    narrate("until you bled.");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate("\nbled...");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate("\nand bled...");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");

    narrate("\nIn the absolute stillness of the midnight,");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    narrate(" you accepted the burden of the...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");
        
    narrate("\n--- CHOOSE YOUR CALLING ---\n"); 
    std::vector<string> classNames;
    for (const auto& c : classDb.templates) classNames.push_back(c.name);

    PagedSelector classSelector(classNames);
    size_t classIndex = classSelector.select();
    const PlayerClassTemplate& chosenClass = classDb.templates[classIndex];

    cout << "\n[ SO BE IT ]" << endl;
    cout << "Identity: " << name << ", a of the " << chosenRace.name <<  " Race" << endl;
    cout << "Class: "<< chosenClass.name << endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    narrate(chosenRace.lore.description, 10);

    cout << "\nAccept this fate? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (confirm == 'n' || confirm == 'N') {
        narrate("Then perhaps it is better to remain in the nothingness.");
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        return 0;
    }


    PlayerRace playerRace{ chosenRace.name, { chosenRace.lore.description } };
    Player hero(name, playerRace, chosenClass);
    hero.applyRaceBonus(chosenRace.statBonus);

    cout << "\nCharacter created successfully!" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    system("cls");

  
    narrate("\nYour chosen race is: " + chosenRace.name + "\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    narrate("\nThe " + chosenClass.name + " is your chosen class. \n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    system("cls");
    cout << "\nPress Enter to step into the grey...\n";
    
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    system("cls");
    mainMenu(hero);
    return 0;
}