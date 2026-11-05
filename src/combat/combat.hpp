#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "core/types.hpp"
#include "catalogs/spell.hpp"
#include "catalogs/equipment.hpp"
#include "entities/npc.hpp"
#include "entities/enemy.hpp"
#include "gameplay/inventory.hpp"
#include "entities/player.hpp"
#include "world/status_effects.hpp"
#include "abilities.hpp"

// Abstract interface for anything that can fight in combat.
class ICombatant {
public:
    virtual std::string getName() const = 0;
    virtual int getPhysicalAttack() const = 0;
    virtual int getMagicAttack() const = 0;
    virtual float getArmor() const = 0;
    virtual float getMagicArmor() const = 0;
    virtual float getCritRate() const = 0;
    virtual float getCritDamage() const = 0;
    virtual float getDodgeRate() const = 0;
    virtual bool isPlayer() const = 0;
    virtual void takeDamage(int amount) = 0;
    virtual const std::vector<ActiveDebuff>& getDebuffs() const = 0;
    virtual std::vector<ActiveDebuff>& mutableDebuffs() = 0;
    // Hidden per-debuff fill toward the next application. Combat-only.
    virtual const std::map<std::string, int>& debuffMeters() const = 0;
    virtual std::map<std::string, int>& mutableDebuffMeters() = 0;
    // Meter resistance derived from armor (0..0.5).
    virtual float debuffResistance() const = 0;
    virtual void applyDebuff(const std::string& debuff) = 0;
    // Debuffs this combatant can inflict with a normal attack.
    virtual const std::vector<std::string>& carriedDebuffs() const {
        static const std::vector<std::string> none;
        return none;
    }
    virtual ~ICombatant() = default;
};

// Combatant adapter for an NPC party member.
class NPCCombatant : public ICombatant {
public:
    NPCCombatant(NPC& n) : npc(n) {}
    std::string getName() const override { return npc.name; }
    int getPhysicalAttack() const override { return npc.stats.attack; }
    int getMagicAttack() const override { return npc.stats.magicAttack; }
    float getArmor() const override { return npc.stats.armor; }
    float getMagicArmor() const override { return npc.stats.magicArmor; }
    float getCritRate() const override { return npc.stats.critRate; }
    float getCritDamage() const override { return npc.stats.critDamage; }
    float getDodgeRate() const override { return npc.stats.dodgeRate; }
    bool isPlayer() const override { return false; }
    void takeDamage(int amount) override { npc.stats.hitpoints -= amount; }
    const std::vector<ActiveDebuff>& getDebuffs() const override { return npc.debuffs; }
    std::vector<ActiveDebuff>& mutableDebuffs() override { return npc.debuffs; }
    const std::map<std::string, int>& debuffMeters() const override { return npc.debuffMeters; }
    std::map<std::string, int>& mutableDebuffMeters() override { return npc.debuffMeters; }
    float debuffResistance() const override {
        if (npc.equippedArmor.empty()) return 0.0f;
        static const EquipmentandWeaponDatabase db;
        for (const auto& eq : db.getEquipment())
            if (eq.name == npc.equippedArmor)
                return statuses::armorResistance(
                    std::max(0, eq.defenseIncrease) / 100.0f);
        return 0.0f;
    }
    void applyDebuff(const std::string& debuff) override { statuses::apply(npc.debuffs, debuff); }

private:
    NPC& npc;
};

// Combatant adapter for the player.
class PlayerCombatant : public ICombatant {
public:
    PlayerCombatant(Player& p) : player(p) {}
    void setInventory(const PlayerInventory& value) { inventory = &value; }
    std::string getName() const override { return player.name; }
    int getPhysicalAttack() const override { return burdened() ? player.stats.attack * 9 / 10 : player.stats.attack; }
    int getMagicAttack() const override { return burdened() ? player.stats.magicAttack * 9 / 10 : player.stats.magicAttack; }
    float getArmor() const override { return player.stats.armor; }
    float getMagicArmor() const override { return player.stats.magicArmor; }
    float getCritRate() const override { return player.stats.critRate; }
    float getCritDamage() const override { return player.stats.critDamage; }
    float getDodgeRate() const override {
        float rate = player.stats.dodgeRate - (burdened() ? 0.10f : 0.0f);
        return rate < 0.0f ? 0.0f : rate;
    }
    bool isPlayer() const override { return true; }
    void takeDamage(int amount) override { player.stats.hitpoints -= amount; }
    const std::vector<ActiveDebuff>& getDebuffs() const override { return player.debuffs; }
    std::vector<ActiveDebuff>& mutableDebuffs() override { return player.debuffs; }
    const std::map<std::string, int>& debuffMeters() const override { return player.debuffMeters; }
    std::map<std::string, int>& mutableDebuffMeters() override { return player.debuffMeters; }
    float debuffResistance() const override { return inventory ? inventory->armorResistance() : 0.0f; }
    void applyDebuff(const std::string& debuff) override { statuses::apply(player.debuffs, debuff); }
    const std::vector<std::string>& carriedDebuffs() const override { return player.equippedWeaponDebuffs; }

private:
    Player& player;
    const PlayerInventory* inventory = nullptr;
    bool burdened() const { return inventory && inventory->encumbered(); }
};

// Combatant adapter for an enemy.
class EnemyCombatant : public ICombatant {
public:
    EnemyCombatant(Enemy& e) : enemy(e) {}
    std::string getName() const override { return enemy.name; }
    int getPhysicalAttack() const override { return enemy.stats.data.attack; }
    int getMagicAttack() const override { return enemy.stats.data.magicAttack; }
    float getArmor() const override { return enemy.stats.data.armor; }
    float getMagicArmor() const override { return enemy.stats.data.magicArmor; }
    float getCritRate() const override { return enemy.stats.data.critRate; }
    float getCritDamage() const override { return enemy.stats.data.critDamage; }
    float getDodgeRate() const override { return enemy.stats.data.dodgeRate; }
    bool isPlayer() const override { return false; }
    void takeDamage(int amount) override { enemy.stats.data.hitpoints -= amount; }
    const std::vector<ActiveDebuff>& getDebuffs() const override { return enemy.debuffs; }
    std::vector<ActiveDebuff>& mutableDebuffs() override { return enemy.debuffs; }
    const std::map<std::string, int>& debuffMeters() const override { return enemy.debuffMeters; }
    std::map<std::string, int>& mutableDebuffMeters() override { return enemy.debuffMeters; }
    float debuffResistance() const override { return statuses::armorResistance(enemy.stats.data.armor); }
    void applyDebuff(const std::string& debuff) override { statuses::apply(enemy.debuffs, debuff); }
    const std::vector<std::string>& carriedDebuffs() const override { return enemy.inflictedDebuffs; }

private:
    Enemy& enemy;
};

// Resolves attacks between combatants.
class CombatSystem {
public:
    CombatResult attack(ICombatant& attacker, ICombatant& target, float damageMultiplier = 1.0f);

private:
    int calculateDamage(int attack, float defense);
};

// Drives the interactive combat loop and rendering.
class CombatScreen {
public:
    CombatScreen(Player& pl, std::vector<NPC>& prty, Enemy& en, TimeSystem& ts, NPCGenerator& gen, SpellDatabase& sdb)
        : player(pl), party(prty), enemy(en), playerC(pl), enemyC(en), attackInfos(), timeSystem(ts), npcGen(gen), spellDB(sdb) {}

    void startCombat(CombatSystem& combat, PlayerInventory& inventory);

private:
    Player& player;
    std::vector<NPC>& party;
    Enemy& enemy;
    PlayerCombatant playerC;
    EnemyCombatant enemyC;
    std::vector<std::string> attackInfos;
    TimeSystem timeSystem;
    NPCGenerator& npcGen;
    SpellDatabase& spellDB;
    AbilityState abilityState;
    // Spell armor bonuses are temporary and stored as percentage points.
    float temporaryArmor = 0.0f;
    float temporaryMagicArmor = 0.0f;

    void tickStatusEffects();
    void displayCombatScreen();
    int getPlayerActionInput() const;
    void handlePlayerAttack(CombatSystem& combat);
    void handleCastSpell();
    void handleEnemyTurn(CombatSystem& combat);
    void removeDeadPartyMembers();
    void displayCombatOutcome() const;
};
