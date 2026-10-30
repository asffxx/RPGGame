# RPGGame

A command-line RPG written in C++.

## Description

A text-based RPG with a bit of depth. You create a character, explore a
hand-crafted-yet-random world, fight enemies, manage a party, and grow stronger
through a global scaling progression system.

## Features

- **Race and Class Selection** — 9 races and 13 classes, each with its own stat spread.
- **Progression with Global Scaling** — level up and scale stats globally, including enemies.
- **Time System** — day to night, advancing every 4 actions (Morning → Afternoon → Evening → Night).
- **Locations, Events, and Enemies** — an interconnected world with random events and enemy encounters.
- **NPC Generation** — randomly generated NPCs with dialogues, stories, and the ability to hire them at the tavern.
- **Party Management** — up to 4 members, with weekly wages deducted from your gold.
- **Stores** — potions, equipment, food, and drinks.
- **Spells and Enchanting** — learn spells and enchant staffs/weapons.
- **Debuffs and Buffs** — temporary effects that modify combat and stats, including stackable debuffs.
- **Travel System** — random exploration to discover new locations (you might stumble into a high-level area early).
- **Inventory** — equipped and enchanted tags, plus food/drink, potion, and equipment handling.
- **Dictionary** — tracks everything you discover: enemies, locations, weapons, events, and special characters.

## Character Creation

The game opens with a title menu (`Start` / `Exit`), then asks for three things
in this order:

1. **Race** — pick from the 9 available races.
2. **Class** — pick from the 13 available classes.
3. **Name** — type your character's name.

You then start in the Abandoned Cell with your race bonus applied on top of your
class stats. There is no confirmation step and no lore narration; choosing is
final.

Menus are driven by numbers: type the option number and press Enter. The only
free-text input is your character's name.

## Combat

Combat is turn-based and renders in phases:

- **Your turn** — choose Attack, Use Item/Potion, Run, or Cast Spell (if you have learned one).
- **Party turn** — each living party member strikes the enemy.
- **Enemy turn** — the enemy retaliates.

After every full turn the screen refreshes once and waits for Enter, so the whole
exchange is visible on one screen before the next turn starts. The main menu
clears and redraws after each action as well.

## Getting Started

### Prerequisites

- A C++17-compatible compiler (`g++`, or MSVC).
- CMake 3.16 or newer.
- Windows, Linux, or macOS. Screen clearing uses `cls` on Windows and `clear` elsewhere.

### Build & Run

**Linux / macOS:**

```sh
cmake -B build -S .
cmake --build build
./build/rpg
```

**Windows (PowerShell or cmd):**

```powershell
cmake -B build -S .
cmake --build build
.\build\rpg.exe
```

CMake adds the `.exe` suffix automatically on Windows. If your CMake picks the
Visual Studio generator, the binary lands in `build\Release\rpg.exe` instead —
build with `cmake --build build --config Release` in that case.

Without CMake, you can compile directly:

```sh
g++ -std=c++17 -Isrc $(find src -name '*.cpp' ! -name 'enemy_data.cpp') -o rpg
```

`src/data/enemy_data.cpp` must be excluded: it is `#include`d by
`src/entities/enemy.cpp`, so compiling it separately causes duplicate symbols.

## Gameplay Loop

The main menu is grouped into **Character**, **Local Establishments**,
**Actions**, and **System**.

1. **Character** — view stats, open your inventory, manage the party, or browse the dictionary.
2. **Explore** — 80% chance of an enemy encounter, 20% chance of a random event.
3. **Travel** — venture into the unknown to discover locations, or take a safe route to ones you have marked.
4. **Visit establishments** — Stores (villages and towns), Taverns (towns), and Magic Stores (spell stores) offer shopping, hiring, resting, and spell learning.
5. **Manage** — keep your inventory, equipment, party, and dictionary in order.
6. **Progress** — defeat enemies for gold and experience, level up, and watch the world scale with you.

Every 4 actions advances the time of day. Traveling into a location and
exploring there advances it too.

### Currency

- 100 Copper = 1 Silver
- 100 Silver = 1 Gold
- 100 Gold = 1 Platinum

### Sleep

Sleeping at a Tavern costs 1 Gold, restores your HP, and advances time.

If you go a full day without sleeping, you lose roughly 5% of your max HP at the
start of the next day.

### Leveling

Level 1 requires 100 experience. Each level multiplies the requirement by 1.2 and
scales your stats by 1.1. Enemies scale with the difficulty of the area you meet
them in.

## Races

Human, Noble, Commoner, Elf, High Elf, Wood Elf, Dark Elf, Orc, Reptilian.

## Classes

Warrior, Knight, Berserker, Assassin, Rogue, Ranger, Mage, Sorcerer, Scholar,
Shrewd Diplomat, Travelling Merchant, Sheltered Noble, Prisoner.

## Project Structure

The codebase was originally a single `main.cpp` and is now split into modules
under `src/`:

```
src/
  app/        main menu loop and all text output
  combat/     combatants, damage calculation, fight loop
  core/       stats, economy, time, UI primitives
  data/       static catalogs (races, classes, items, enemies, events)
  entities/   Player, Enemy, NPC and their generators
  systems/    inventory, events, party, dictionary
  ui/         menu rendering and the player stat panel
  world/      travel and establishments
```

## Disclaimer

Certain classes such as `NPC`, `Player`, and `EnemyCombatant` were aided by AI. The `PlayerCombatant` class was AI-aided but integrated into the rest of the codebase. The combat damage formula is AI-aided since I can't think of a formula.

## Acknowledgements

Basic RPG mechanics. Some contents were inspired by other games.
