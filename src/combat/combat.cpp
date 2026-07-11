#include "combat.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "core/ui.hpp"
#include "app/display.hpp"

namespace {

// Player-selectable actions in the combat menu.
enum class CombatAction { Attack = 1, UseItem = 2, Run = 3, CastSpell = 4 };

// Renders a fixed-width HP bar, e.g. "[██████░░░░]".
std::string makeHpBar(int hp, int maxHp, int width = 18) {
    if (maxHp <= 0) maxHp = 1;
    int filled = static_cast<int>(std::round(static_cast<double>(hp) / maxHp * width));
    filled = std::max(0, std::min(width, filled));
    return "[" + std::string(filled, '#') + std::string(width - filled, '-') + "]";
}

// Builds a comma-separated debuff line, or an empty string when none.
std::string makeDebuffLine(const std::vector<std::string>& debuffs) {
    if (debuffs.empty()) return "";
    std::string line = "Debuffs: ";
    for (size_t i = 0; i < debuffs.size(); ++i) {
        if (i > 0) line += ", ";
        line += debuffs[i];
    }
    return line;
}

// Describes a single attack result as a combat-log line.
std::string describeAttack(const CombatResult& result) {
    std::string info = result.attackerName + " attacked " + result.targetName;
    if (result.dodge) {
        info += " but " + result.targetName + " dodged!";
    } else {
        info += " and dealt " + std::to_string(result.totalDamage) + " damage";
        if (result.isCrit) info += " (Critical Hit!)";
        info += ".";
    }
    return info;
}

// Waits for the user to press Enter exactly once. Combat reads the player's
// action with getline (which already consumes its trailing newline), so there
// is normally no leftover newline to discard. We only swallow a leftover '\n'
// if one is already sitting in the buffer, without blocking for new input;
// otherwise the first Enter the user presses would be consumed as that leftover
// and a second press would be required.
void waitForEnter() {
    if (std::cin.rdbuf()->in_avail() > 0 && std::cin.peek() == '\n') {
        std::cin.ignore();
    }
    display::pressEnter();
}

} // namespace

CombatResult CombatSystem::attack(ICombatant& attacker, ICombatant& target) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    bool dodge = dist(gen) < target.getDodgeRate();

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

        auto action = static_cast<CombatAction>(getPlayerActionInput());
        switch (action) {
            case CombatAction::Attack:
                handlePlayerAttack(combat);
                break;
            case CombatAction::UseItem:
                inventory.showInventory(player);
                break;
            case CombatAction::Run:
                display::combatRunAway();
                inCombat = false;
                continue;
            case CombatAction::CastSpell:
                handleCastSpell();
                break;
            default:
                display::combatInvalidChoice();
                break;
        }

        // Living party members strike in turn; the dead are skipped and
        // removed after the enemy's retaliation.
        for (auto it = party.begin(); it != party.end(); ) {
            if (it->stats.hitpoints <= 0) {
                ++it;
                continue;
            }
            NPCCombatant npcC(*it);
            CombatResult result = combat.attack(npcC, enemyC);
            attackInfos.push_back(describeAttack(result));
            ++it;
        }

        handleEnemyTurn(combat);
        system("cls");
        displayCombatScreen();
        waitForEnter();

        inventory.tickBuffs(player);
        removeDeadPartyMembers();
    }

    displayCombatOutcome();
}

void CombatScreen::displayCombatScreen() {
    std::vector<std::string> lines;

    lines.push_back("Your Party:");
    lines.push_back(player.name + "  " + makeHpBar(player.stats.hitpoints, player.stats.maxHitpoints) + " " +
                    std::to_string(player.stats.hitpoints) + "/" + std::to_string(player.stats.maxHitpoints) +
                    "   MP " + std::to_string(player.stats.mana) + "/" + std::to_string(player.stats.maxMana));
    std::string playerDebuffs = makeDebuffLine(player.debuffs);
    if (!playerDebuffs.empty()) lines.push_back("  " + playerDebuffs);

    for (const auto& npc : party) {
        lines.push_back(npc.name + "  " + makeHpBar(npc.stats.hitpoints, npc.stats.maxHitpoints) + " " +
                        std::to_string(npc.stats.hitpoints) + "/" + std::to_string(npc.stats.maxHitpoints));
        std::string npcDebuffs = makeDebuffLine(npc.debuffs);
        if (!npcDebuffs.empty()) lines.push_back("  " + npcDebuffs);
    }

    lines.push_back("");
    lines.push_back("Enemy:");
    lines.push_back(enemy.name + "  " + makeHpBar(enemy.stats.data.hitpoints, enemy.stats.data.maxHitpoints) + " " +
                    std::to_string(enemy.stats.data.hitpoints) + "/" + std::to_string(enemy.stats.data.maxHitpoints));
    std::string enemyDebuffs = makeDebuffLine(enemy.debuffs);
    if (!enemyDebuffs.empty()) lines.push_back("  " + enemyDebuffs);

    if (!attackInfos.empty()) {
        lines.push_back("");
        lines.push_back("--- Combat Log ---");
        for (const auto& info : attackInfos) {
            lines.push_back(info);
        }
    }

    displayBorderedMenu(lines, "");
    attackInfos.clear();
}

int CombatScreen::getPlayerActionInput() const {
    std::vector<std::string> actions = {"Attack", "Use Item / Potion", "Run"};
    if (!player.learnedSpells.empty()) {
        actions.push_back("Cast Spell");
    }
    displayBorderedMenu(actions, "Choose your action: ");
    return getNumberInput(1, static_cast<int>(actions.size()));
}

void CombatScreen::handlePlayerAttack(CombatSystem& combat) {
    CombatResult result = combat.attack(playerC, enemyC);
    attackInfos.push_back(describeAttack(result));
}

void CombatScreen::handleCastSpell() {
    if (player.learnedSpells.empty()) return;

    display::combatSpellPrompt(player.learnedSpells);
    int choice = getNumberInput(1, static_cast<int>(player.learnedSpells.size()));
    std::string spellName = player.learnedSpells[choice - 1];

    const auto& spells = spellDB.getSpells();
    auto it = std::find_if(spells.begin(), spells.end(), [&](const SpellDatabase::SpellData& s){ return s.spellName == spellName; });
    if (it == spells.end()) return;
    const auto& spell = *it;

    if (player.stats.mana < spell.manaCost) {
        display::combatManaInsufficient();
        return;
    }

    player.stats.mana -= spell.manaCost;
    std::string info = player.name + " cast " + spell.spellName;

    if (spell.healthDamage > 0) {
        enemy.stats.data.hitpoints -= spell.healthDamage;
        info += " and dealt " + std::to_string(spell.healthDamage) + " damage";
    }
    if (spell.manaDamage > 0) {
        enemy.stats.data.mana = std::max(0, enemy.stats.data.mana - spell.manaDamage);
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
    attackInfos.push_back(describeAttack(result));
}

void CombatScreen::removeDeadPartyMembers() {
    for (auto it = party.begin(); it != party.end(); ) {
        if (it->stats.hitpoints <= 0) {
            npcGen.unlockName(it->name);
            it = party.erase(it);
        } else {
            ++it;
        }
    }
}

void CombatScreen::displayCombatOutcome() const {
    if (player.stats.hitpoints <= 0) {
        display::combatDefeat(player.name);
    } else if (enemy.stats.data.hitpoints <= 0) {
        display::combatVictory(enemy.name);
    }
}
