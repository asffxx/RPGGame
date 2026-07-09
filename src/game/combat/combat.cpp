#include "combat.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <random>

#include "core/ui.hpp"

// Waits for the user to press Enter. Consumes any leftover newline left in the
// input stream by a preceding numeric read so the pause doesn't get skipped.
static void waitForEnter() {
    if (std::cin.peek() == '\n') {
        std::cin.ignore();
    }
    std::cout << "\nPress Enter to continue...\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

CombatResult CombatSystem::attack(ICombatant& attacker, ICombatant& target) {
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
    std::string debuffInflicted = "";

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

int CombatSystem::calculateDamage(int attack, float defense) {
    float baseDamage = static_cast<float>(attack) * (1.0f - defense);
    static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> dist(-baseDamage * 0.15f, baseDamage * 0.15f);
    float damage = baseDamage + dist(gen);
    return static_cast<int>(std::round(std::max<float>(damage, 0.0f)));
}

void CombatScreen::startCombat(CombatSystem& combat, PlayerInventory& inventory) {
    bool inCombat = true;

    while (inCombat && (player.stats.hitpoints > 0 || !party.empty()) && enemy.stats.data.hitpoints > 0) {
        system("cls");
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
                std::cout << "You ran away!" << std::endl;
                inCombat = false;
                continue;
            case 4:
                handleCastSpell();
                break;
            default:
                std::cout << "Invalid choice!" << std::endl;
                break;
        }

        for (auto it = party.begin(); it != party.end(); ) {
            if (it->stats.hitpoints > 0) {
                NPCCombatant npcC(*it);
                CombatResult result = combat.attack(npcC, enemyC);
                std::string info = result.attackerName + " attacked " + result.targetName;
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
        system("cls");
        displayCombatScreen();
        waitForEnter();

        inventory.tickBuffs(player);

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

void CombatScreen::displayCombatScreen() {
    std::vector<std::string> lines;
    lines.push_back(player.name + " - HP: " + std::to_string(player.stats.hitpoints) + "/" + std::to_string(player.stats.maxHitpoints) + " MP: " + std::to_string(player.stats.mana) + "/" + std::to_string(player.stats.maxMana));
    if (!player.debuffs.empty()) {
        std::string debuffs = "Debuffs: ";
        for (const auto& debuff : player.debuffs) {
            debuffs += debuff + " ";
        }
        lines.push_back(debuffs);
    }
    for (const auto& npc : party) {
        lines.push_back(npc.name + " - HP: " + std::to_string(npc.stats.hitpoints) + "/" + std::to_string(npc.stats.maxHitpoints));
        if (!npc.debuffs.empty()) {
            std::string debuffs = "Debuffs: ";
            for (const auto& debuff : npc.debuffs) {
                debuffs += debuff + " ";
            }
            lines.push_back(debuffs);
        }
    }
    lines.push_back(enemy.name + " - HP: " + std::to_string(enemy.stats.data.hitpoints) + "/" + std::to_string(enemy.stats.data.maxHitpoints));
    if (!enemy.debuffs.empty()) {
        std::string debuffs = "Debuffs: ";
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

int CombatScreen::getPlayerActionInput() const {
    std::cout << "\nChoose your action:" << std::endl;
    std::cout << "1. Attack" << std::endl;
    std::cout << "2. Use Item / Potion" << std::endl;
    std::cout << "3. Run" << std::endl;
    if (!player.learnedSpells.empty()) {
        std::cout << "4. Cast Spell" << std::endl;
        return getNumberInput(1, 4);
    }

    return getNumberInput(1, 3);
}

void CombatScreen::handlePlayerAttack(CombatSystem& combat) {
    CombatResult result = combat.attack(playerC, enemyC);
    std::string info = result.attackerName + " attacked " + result.targetName;
    if (result.dodge) {
        info += " but " + result.targetName + " dodged!";
    } else {
        info += " and dealt " + std::to_string(result.totalDamage) + " damage" + (result.isCrit ? " (Critical Hit!)" : "") + ".";
    }
    attackInfos.push_back(info);
}

void CombatScreen::handleCastSpell() {
    if (player.learnedSpells.empty()) return;

    std::cout << "Choose a spell to cast:" << std::endl;
    for (size_t i = 0; i < player.learnedSpells.size(); ++i) {
        std::cout << i + 1 << ". " << player.learnedSpells[i] << std::endl;
    }
    int choice = getNumberInput(1, static_cast<int>(player.learnedSpells.size()));
    std::string spellName = player.learnedSpells[choice - 1];

    const auto& spells = spellDB.getSpells();
    auto it = std::find_if(spells.begin(), spells.end(), [&](const SpellDatabase::SpellData& s){ return s.spellName == spellName; });
    if (it == spells.end()) return;
    const auto& spell = *it;

    if (player.stats.mana < spell.manaCost) {
        std::cout << "Mana insufficient" << std::endl;
        return;
    }

    player.stats.mana -= spell.manaCost;
    std::string info = player.name + " cast " + spell.spellName;

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

void CombatScreen::handleEnemyTurn(CombatSystem& combat) {
    if (enemy.stats.data.hitpoints <= 0 || player.stats.hitpoints <= 0)
        return;
    CombatResult result = combat.attack(enemyC, playerC);
    std::string info = result.attackerName + " attacked " + result.targetName;
    if (result.dodge) {
        info += " but " + result.targetName + " dodged!";
    } else {
        info += " and dealt " + std::to_string(result.totalDamage) + " damage" + (result.isCrit ? " (Critical Hit!)" : "") + ".";
    }
    attackInfos.push_back(info);
}

void CombatScreen::displayCombatOutcome() const {
    if (player.stats.hitpoints <= 0)
        std::cout << player.name << " has been defeated!" << std::endl;
    else if (enemy.stats.data.hitpoints <= 0)
        std::cout << enemy.name << " has been defeated!" << std::endl;
}
