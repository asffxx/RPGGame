# RPGGame

A command-line RPG written in C++.

## Description

My attempt at making an RPG at C++ with slight depth. You create a character, explore a
hand-crafted-yet-random world, fight enemies, manage a party, and grow stronger through a
global scaling progression system.

## Features

- **Player Class and Race Selection**: Choose from 9 races and 13 classes to customize your character.
- **Progression with Global Scaling**: Level up and scale stats globally, including enemies.
- **Time System**: Day to night, advancing every 4 turns (Morning → Afternoon → Evening → Night).
- **Locations, Events, and Enemies**: An interconnected world with random events and enemy encounters.
- **NPC Generation**: Randomly generated NPCs with inventories (not viewable), dialogues, stories, and the ability to hire them at the tavern.
- **Party Management System**: Manage a party of up to 4 members, including weekly wage deductions. Manage your gold or they will leave.
- **Store System**: Diverse item sets including potions, equipment, food, and drinks.
- **Spell and Enchanting Features**: Learn spells and enchant staffs/weapons for enhanced abilities.
- **Debuff and Buff System**: Temporary effects that impact combat and stats, including stackable debuffs.
- **Travel System**: Random exploration to discover new locations (truly random — you might stumble into a high-level area early).
- **Basic Inventory Handling**: Equipped and Enchanted tags, plus food/drink, potion, and equipment management.
- **Dictionary**: Track everything you discover — enemies, locations, weapons, events, and special characters.

## Character Creation

The intro is a focused, narrated flow:

1. **Select your ancestry** — choose a race from the catalog.
2. **Name your character** — enter a name when prompted.
3. **Read the lore** — your chosen race's description is narrated to you.
4. **Confirm your fate** — accept (y) or reject (n) the selection. Rejecting exits the game.
5. **Choose your calling** — after accepting, pick a class to finalize the character.

Class selection happens after the race/name/lore intro so the opening stays centered
on the character's identity and backstory.

## Combat

Combat is turn-based and renders in phases:

- **Your turn** — choose Attack, Use Item/Potion, Run, or Cast Spell (if learned).
- **Party turn** — each living party member strikes the enemy.
- **Enemy turn** — the enemy retaliates.

After every full turn (your action, party actions, and the enemy's action) the screen
refreshes once and waits for you to press Enter, so the whole exchange is visible on a
single screen before the next turn begins. The main menu clears and redraws after each
action as well.

## Getting Started

### Prerequisites

- A C++17-compatible compiler (the project is built with `g++` from MSYS2/MinGW-w64).
- Windows (uses `system("cls")` for screen clearing).

### Build

The game is now split into modular translation units under `src/`. Compile all
sources together (the entry point is `src/main.cpp`):

```sh
g++ -std=c++17 -O2 -I src \
  src/main.cpp \
  src/core/*.cpp src/db/*.cpp src/npc/*.cpp src/game/*.cpp src/world/*.cpp \
  -o rpg.exe
```

A `CMakeLists.txt` is also provided:

```sh
cmake -B build && cmake --build build
```

> Note: `src/game/enemy_data.cpp` is intentionally `#include`d at the end of
> `src/game/enemy.cpp` to populate the enemy template catalog; do not add it as a separate compilation unit.

### Run

```sh
rpg.exe
```

At the opening prompt, type `Quick Start` to skip character creation and jump straight into
the game with a default character.

## Gameplay Loop

1. **Create your character** — pick a name and race, read your race's lore, then choose a class to begin.
2. **Explore** — venture out to fight enemies (80% chance) or trigger random events (20% chance). Every 4 actions advances the time of day.
3. **Travel** — move between locations to discover new ones. Difficulty scales with the area.
4. **Visit establishments** — Stores (villages/towns), Taverns (towns), and Magic Stores (spell stores) offer shopping, hiring, resting, and spell-learning.
5. **Manage** — keep your inventory, equipment, party wages, and dictionary in order.
6. **Progress** — defeat enemies for gold and experience, level up, and watch the world scale with you.

### Currency

- 100 Copper = 1 Silver
- 100 Silver = 1 Gold
- 100 Gold = 1 Platinum

### Sleep

If you don't sleep (at a Tavern) you lose roughly 5% of your HP at the start of each new day.

## Races

Human, Noble, Commoner, Elf, High Elf, Wood Elf, Dark Elf, Orc, Reptilian.

## Classes

Warrior, Knight, Berserker, Assassin, Rogue, Ranger, Mage, Sorcerer, Scholar,
Shrewd Diplomat, Travelling Merchant, Sheltered Noble, Prisoner.

> The codebase was refactored from a single file `main.cpp`. Behavior is preserved; only structure changed.

## Disclaimer

Certain classes such as `NPC`, `Player`, and `EnemyCombatant` were aided by AI. The `PlayerCombatant` class was AI-aided but integrated into the rest of the codebase. The combat damage formula is AI-aided since I can't think of a formula.

## Acknowledgements

Basic RPG mechanics. Some contents were inspired by other games.
