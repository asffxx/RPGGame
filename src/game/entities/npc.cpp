#include "npc.hpp"

#include <algorithm>
#include <random>
#include "db/class.hpp"
#include "db/equipment.hpp"
#include "db/race.hpp"
#include "db/spell.hpp"

NPCGenerator::NPCGenerator()
    : firstNames({"Godfrey","Augustus","Edmund","Alfred","Theodore","Marcus","Julius","Lucius","Constantine","Benedict","Sebastian","Cornelius","Ambrose","Gregory","Leonard","Matthias","Philip","Alexander","Dominic","Victor","Hugh","Gerald","Roland","Bernard","Percival"}),
      lastNames({"Janus","Godwin","Edwards","Aurelian","Constantinus","Benedictus","Marcellus","Valerian","Justinian","Hadrian","Maximus","Cassius","Severus","Flavian","Gratian","Laurentius","Paulinus","Victorinus","Dominicus","Magnus","Theodoric","Regulus","Claudius","Tiberius","Germanicus"}),
      specialCharacters({"Evelyn Chevalier", "Astra Yao", "Ye Shunguang ", "Burnice White","Jane Doe", "Belle"}) {}

void NPCGenerator::lockName(const std::string& name) {
    lockedNames.insert(name);
}

void NPCGenerator::unlockName(const std::string& name) {
    lockedNames.erase(name);
}

NPC NPCGenerator::generateNPC(int playerLevel) {
    static std::mt19937 gen(std::random_device{}());

    int minLevel = std::max(1, playerLevel - 2);
    int maxLevel = playerLevel + 2;
    std::uniform_int_distribution<int> levelDist(minLevel, maxLevel);
    int level = levelDist(gen);

    std::uniform_real_distribution<float> specialDist(0.0f, 1.0f);
    bool isSpecial = specialDist(gen) < 0.1f; // 10% chance

    std::string name;
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



   std::vector<std::string> stories = {
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
    std::vector<std::string> availableSpells;
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
        std::vector<std::string> normalDialogues = {
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
