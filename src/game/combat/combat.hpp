#pragma once

#include <string>
#include <vector>

#include "core/types.hpp"
#include "db/spell.hpp"
#include "game/entities/npc.hpp"
#include "game/entities/enemy.hpp"
#include "game/systems/inventory.hpp"
#include "game/entities/player.hpp"

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
    virtual const std::vector<std::string>& getDebuffs() const = 0;
    virtual void applyDebuff(const std::string& debuff) = 0;
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
    const std::vector<std::string>& getDebuffs() const override { return npc.debuffs; }
    void applyDebuff(const std::string& debuff) override { npc.debuffs.push_back(debuff); }

private:
    NPC& npc;
};

// Combatant adapter for the player.
class PlayerCombatant : public ICombatant {
public:
    PlayerCombatant(Player& p) : player(p) {}
    std::string getName() const override { return player.name; }
    int getPhysicalAttack() const override { return player.stats.attack; }
    int getMagicAttack() const override { return player.stats.magicAttack; }
    float getArmor() const override { return player.stats.armor; }
    float getMagicArmor() const override { return player.stats.magicArmor; }
    float getCritRate() const override { return player.stats.critRate; }
    float getCritDamage() const override { return player.stats.critDamage; }
    float getDodgeRate() const override { return player.stats.dodgeRate; }
    bool isPlayer() const override { return true; }
    void takeDamage(int amount) override { player.stats.hitpoints -= amount; }
    const std::vector<std::string>& getDebuffs() const override { return player.debuffs; }
    void applyDebuff(const std::string& debuff) override { player.debuffs.push_back(debuff); }
    const std::vector<std::string>& getEquippedWeaponDebuffs() const { return player.equippedWeaponDebuffs; }
    float getEquippedWeaponDebuffChance() const { return player.equippedWeaponDebuffChance; }

private:
    Player& player;
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
    const std::vector<std::string>& getDebuffs() const override { return enemy.debuffs; }
    void applyDebuff(const std::string& debuff) override { enemy.debuffs.push_back(debuff); }

private:
    Enemy& enemy;
};

// Resolves attacks between combatants.
class CombatSystem {
public:
    CombatResult attack(ICombatant& attacker, ICombatant& target);

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

    void displayCombatScreen();
    int getPlayerActionInput() const;
    void handlePlayerAttack(CombatSystem& combat);
    void handleCastSpell();
    void handleEnemyTurn(CombatSystem& combat);
    void displayCombatOutcome() const;
};
