#include "combat.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <string>
#include <vector>

#include "core/ui.hpp"
#include "app/display.hpp"

namespace {

// Fills the target's per-debuff meters for every debuff the attacker
// carries. A meter that fills applies its debuff and empties; the names of
// the debuffs applied are returned.
std::vector<std::string> fillDebuffMeters(ICombatant& attacker, ICombatant& target) {
    const float resist = target.debuffResistance();
    std::vector<std::string> inflicted;
    for (const auto& debuff : attacker.carriedDebuffs())
        if (statuses::addDebuffMeter(target.mutableDebuffMeters(), debuff, resist)) {
            std::string message;
            if (statuses::apply(target.mutableDebuffs(), debuff, &message))
                inflicted.push_back(debuff);
        }
    return inflicted;
}

// Player-selectable actions in the combat menu.
enum class CombatAction { Attack = 1, UseItem = 2, Run = 3, Signature = 4, CastSpell = 5 };

// Renders a fixed-width HP bar, e.g. "[██████░░░░]".
std::string makeHpBar(int hp, int maxHp, int width = 18) {
    if (maxHp <= 0) maxHp = 1;
    int filled = static_cast<int>(std::round(static_cast<double>(hp) / maxHp * width));
    filled = std::max(0, std::min(width, filled));
    return "[" + std::string(filled, '#') + std::string(width - filled, '-') + "]";
}

// Builds a debuff line with stacks and remaining rounds, or an empty string.
std::string makeDebuffLine(const std::vector<ActiveDebuff>& debuffs) {
    if (debuffs.empty()) return "";
    return "Debuffs: " + statuses::describe(debuffs);
}

// Builds a meter line ("Meters: Burning [##--------] 20"), or an empty string.
std::string makeMeterLine(const std::map<std::string, int>& meters) {
    const std::string description = statuses::describeDebuffMeters(meters);
    return description.empty() ? std::string{} : "Meters: " + description;
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
        if (!result.debuffInflicted.empty())
            info += " It inflicted " + result.debuffInflicted + ".";
    }
    return info;
}

// Numeric menus use getline, and inventory consumes its formatted-input newline.
// The next newline is the user's acknowledgement, including with queued input.
void waitForEnter() {
    display::pressEnter();
}

} // namespace

CombatResult CombatSystem::attack(ICombatant& attacker, ICombatant& target, float damageMultiplier) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    bool dodge = dist(gen) < target.getDodgeRate();

    int physicalDamage = 0;
    int magicalDamage = 0;
    int totalDamage = 0;
    bool isCrit = false;
    std::string debuffInflicted = "";

    if (!dodge) {
        int attack = std::max(0, attacker.getPhysicalAttack() -
                                 statuses::attackPenalty(attacker.getDebuffs()));
        int magicAttack = std::max(0, attacker.getMagicAttack() -
                                      statuses::magicAttackPenalty(attacker.getDebuffs()));
        float armor = std::max(0.0f, target.getArmor() *
                                         statuses::armorMultiplier(target.getDebuffs()));
        float magicArmor = std::max(0.0f, target.getMagicArmor() *
                                              statuses::magicArmorMultiplier(target.getDebuffs()));
        physicalDamage = calculateDamage(attack, std::min(armor, 1.0f));
        magicalDamage = calculateDamage(magicAttack, std::min(magicArmor, 1.0f));
        totalDamage = physicalDamage + magicalDamage;

        if (dist(gen) < attacker.getCritRate()) {
            isCrit = true;
            totalDamage = static_cast<int>(std::round(totalDamage * attacker.getCritDamage()));
        }

        totalDamage = static_cast<int>(std::round(
            totalDamage * damageMultiplier * statuses::incomingDamageMultiplier(target.getDebuffs())));
        target.takeDamage(totalDamage);

        const std::vector<std::string> inflicted = fillDebuffMeters(attacker, target);
        for (size_t i = 0; i < inflicted.size(); ++i) {
            if (!debuffInflicted.empty()) debuffInflicted += ", ";
            debuffInflicted += inflicted[i];
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
    playerC.setInventory(inventory);
    bool inCombat = true;
    abilityState = {};
    temporaryArmor = 0.0f;
    temporaryMagicArmor = 0.0f;
    statuses::clear(player.debuffs);
    statuses::clear(enemy.debuffs);
    for (auto& npc : party) statuses::clear(npc.debuffs);
    player.debuffMeters.clear();
    enemy.debuffMeters.clear();
    for (auto& npc : party) npc.debuffMeters.clear();

    while (inCombat && (player.stats.hitpoints > 0 || !party.empty()) && enemy.stats.data.hitpoints > 0) {
        clearScreen();
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
            case CombatAction::Signature: {
                static std::mt19937 gen(std::random_device{}());
                std::uniform_real_distribution<float> roll(0.0f, 1.0f);
                std::string log;
                if (!useSignature(player, party, enemy, abilityState, roll(gen), roll(gen), log)) {
                    display::message("Signature ability unavailable this encounter.");
                    continue;
                }
                attackInfos.push_back(log);
                break;
            }
            default:
                display::combatInvalidChoice();
                break;
        }

        // Living party members strike in turn; the dead are skipped and
        // removed after the enemy's retaliation.
        for (auto it = party.begin(); it != party.end(); ) {
            if (enemy.stats.data.hitpoints <= 0) break;
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

        const std::string buffExpired = inventory.tickBuffs(player);
        if (!buffExpired.empty()) attackInfos.push_back(buffExpired);
        tickStatusEffects();
        removeDeadPartyMembers();

        clearScreen();
        displayCombatScreen();
        waitForEnter();
    }

    // Combat statuses last for the encounter only. Spell armor bonuses are also
    // temporary, so they cannot leak into saved stats.
    statuses::clear(player.debuffs);
    statuses::clear(enemy.debuffs);
    for (auto& npc : party) statuses::clear(npc.debuffs);
    player.debuffMeters.clear();
    enemy.debuffMeters.clear();
    for (auto& npc : party) npc.debuffMeters.clear();
    if (temporaryArmor > 0) {
        player.stats.armor = std::max(0.0f, player.stats.armor - temporaryArmor);
        temporaryArmor = 0.0f;
    }
    if (temporaryMagicArmor > 0) {
        player.stats.magicArmor = std::max(0.0f, player.stats.magicArmor - temporaryMagicArmor);
        temporaryMagicArmor = 0.0f;
    }
    const auto wearMessages = inventory.wearEquipment(player, 3);

    displayCombatOutcome();
    for (const auto& message : wearMessages) display::message(message);
}

void CombatScreen::tickStatusEffects() {
    auto tickOne = [&](const std::string& name, std::vector<ActiveDebuff>& active,
                       int& hitpoints, int& mana) {
        for (const auto& line : statuses::tick(active, hitpoints, mana))
            attackInfos.push_back(name + " " + line);
    };
    tickOne(player.name, player.debuffs, player.stats.hitpoints, player.stats.mana);
    for (auto& npc : party)
        tickOne(npc.name, npc.debuffs, npc.stats.hitpoints, npc.stats.mana);
    tickOne(enemy.name, enemy.debuffs, enemy.stats.data.hitpoints, enemy.stats.data.mana);
    statuses::decayDebuffMeters(player.debuffMeters);
    for (auto& npc : party) statuses::decayDebuffMeters(npc.debuffMeters);
    statuses::decayDebuffMeters(enemy.debuffMeters);
}

void CombatScreen::displayCombatScreen() {
    std::vector<std::string> lines;

    lines.push_back("[ Combat ]");
    lines.push_back("[ Your Party ]");
    lines.push_back(player.name + "  " + makeHpBar(player.stats.hitpoints, player.stats.maxHitpoints) + " " +
                    std::to_string(std::max(0, player.stats.hitpoints)) + "/" + std::to_string(player.stats.maxHitpoints) +
                    "   MP " + std::to_string(player.stats.mana) + "/" + std::to_string(player.stats.maxMana));
    std::string playerDebuffs = makeDebuffLine(player.debuffs);
    if (!playerDebuffs.empty()) lines.push_back("  " + playerDebuffs);
    const std::string playerMeters = makeMeterLine(player.debuffMeters);
    if (!playerMeters.empty()) lines.push_back("  " + playerMeters);

    for (const auto& npc : party) {
        lines.push_back(npc.name + "  " + makeHpBar(npc.stats.hitpoints, npc.stats.maxHitpoints) + " " +
                        std::to_string(std::max(0, npc.stats.hitpoints)) + "/" + std::to_string(npc.stats.maxHitpoints));
        std::string npcDebuffs = makeDebuffLine(npc.debuffs);
        if (!npcDebuffs.empty()) lines.push_back("  " + npcDebuffs);
        const std::string npcMeters = makeMeterLine(npc.debuffMeters);
        if (!npcMeters.empty()) lines.push_back("  " + npcMeters);
    }

    lines.push_back("");
    lines.push_back("[ Enemy ]");
    lines.push_back(enemy.name + "  " + makeHpBar(enemy.stats.data.hitpoints, enemy.stats.data.maxHitpoints) + " " +
                    std::to_string(std::max(0, enemy.stats.data.hitpoints)) + "/" + std::to_string(enemy.stats.data.maxHitpoints));
    std::string enemyDebuffs = makeDebuffLine(enemy.debuffs);
    if (!enemyDebuffs.empty()) lines.push_back("  " + enemyDebuffs);
    const std::string enemyMeters = makeMeterLine(enemy.debuffMeters);
    if (!enemyMeters.empty()) lines.push_back("  " + enemyMeters);

    if (!attackInfos.empty()) {
        lines.push_back("");
        lines.push_back("[ Combat Log ]");
        for (const auto& info : attackInfos) {
            lines.push_back(info);
        }
    }

    displayBorderedMenu(lines, "");
    attackInfos.clear();
}

int CombatScreen::getPlayerActionInput() const {
    const auto& ability = signatureAbility(player.playerClass.name);
    std::vector<std::string> actions = {"1. Attack", "2. Use Item / Potion", "3. Run",
        "4. " + std::string(ability.name) + (abilityState.used ? " [used]" : " [1 use]")};
    if (!player.learnedSpells.empty()) {
        actions.push_back("5. Cast Spell");
    }
    std::vector<std::string> menu = {std::string(ability.name) + ": " + ability.description,
                                    "----------------------------"};
    menu.insert(menu.end(), actions.begin(), actions.end());
    displayBorderedMenu(menu, "Choose your action: ");
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
        float modifier = spell.hasDebuff && spell.debuffModifierOnHit > 0.0f
            ? spell.debuffModifierOnHit : 1.0f;
        int damage = static_cast<int>(std::round(spell.healthDamage * modifier));
        enemy.stats.data.hitpoints -= damage;
        info += " and dealt " + std::to_string(damage) + " damage";
    }
    if (spell.armorReduction > 0) {
        enemy.stats.data.armor = std::max(0.0f, enemy.stats.data.armor -
                                               static_cast<float>(spell.armorReduction) / 100.0f);
        info += " and broke armor";
    }
    if (spell.magicArmorReduction > 0) {
        enemy.stats.data.magicArmor = std::max(0.0f, enemy.stats.data.magicArmor -
                                                    static_cast<float>(spell.magicArmorReduction) / 100.0f);
        info += " and broke magic armor";
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
        player.stats.armor += spell.armorIncrease / 100.0f;
        temporaryArmor += spell.armorIncrease / 100.0f;
        info += " and increased armor";
    }
    if (spell.magicArmorIncrease > 0) {
        player.stats.magicArmor += spell.magicArmorIncrease / 100.0f;
        temporaryMagicArmor += spell.magicArmorIncrease / 100.0f;
        info += " and increased magic armor";
    }
    if (spell.hasDebuff) {
        const float resist = enemyC.debuffResistance();
        for (const auto& debuff : spell.debuffs) {
            if (!statuses::addDebuffMeter(enemy.debuffMeters, debuff, resist)) continue;
            if (statuses::apply(enemy.debuffs, debuff))
                info += " and applied " + debuff;
        }
    }

    attackInfos.push_back(info + ".");
}

void CombatScreen::handleEnemyTurn(CombatSystem& combat) {
    if (enemy.stats.data.hitpoints <= 0 || player.stats.hitpoints <= 0)
        return;
    float multiplier = abilityState.consumeRetaliation();
    if (multiplier == 0.0f) {
        attackInfos.push_back(enemy.name + " was distracted and could not act.");
        return;
    }
    CombatResult result = combat.attack(enemyC, playerC, multiplier);
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
    // Victory spoils are shown by the caller's drop/gold report; defeat keeps its own screen.
    if (player.stats.hitpoints <= 0) {
        display::combatDefeat(player.name);
    }
}
