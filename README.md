# RPGGame
A command-line based game written in C++

## Description
My attempt at making an RPG Game at C++ with slight depth.

## Features

- **Player Class and Race Selection**: Choose from various races and classes to customize your character.
- **Progression with Global Scaling**: Level up and scale stats globally including enemies.
- **Time System**: Day to night, per 4 turns.
- **Locations, Events, and Enemies**: Interconnected world with random events and enemy encounters.
- **NPC Generation**: Randomly generated NPCs with inventories(not viewable), dialogues, stories, and the ability to hire them at the tavern.
- **Party Management System**: Manage a party of up to 4 members, including wage deductions, try to manage your gold or uh they will leave.
- **Store System**: Diverse item sets including potions, equipment, food, and drinks.
- **Spell and Enchanting Features**: Learn spells and enchant staffs for enhanced abilities.
- **Debuff and Buff System**: Temporary effects that impact combat and stats.
- **Travel System**: Random exploration to discover new locations (Its truly random you might get hit with a high level area on early).
- **Basic Inventory Handling**: Equipped and Enchanted Tag.
- **Dictionary**: Track everything!

## Getting the Code
You can obtain the code in one of two ways:

1. **Clone the Repository**: If you have Git installed, clone the repository using:

   ```
   git clone <https://github.com/asffxx/RPGGame>
   ```

2. Install the zip in the [releases page](https://github.com/asffxx/RPGGame/releases). Extract, and launch <b>rpg.exe<b>. 
<br><br>
### Building the Game
   
1. Once you cloned the repository, open a command prompt in the project directory. 
  
2. Compile the code:
   ```
   g++ src\main.cpp -o rpg.exe
   ```
3. Run the executable:
   ```
   rpg.exe
   ```

## How to Play

- **Navigation**: Use numerical inputs for convenience. Enter numbers to select menu options, actions, and choices.<br><br>
- **Objectives**: Explore locations, fight enemies, manage your party, and progress through the game world. Visit stores, taverns, and magic shops in towns to buy items, rest, or hire some lackeys(NPCS).<br>

**Gameplay Mechanics**:
  - Select your race and class at the start.
  - Explore to encounter enemies or events.
  - Manage time, as not sleeping can cause health penalties.
  - Build a party, equip items, and cast spells in combat.
  - Travel to discover new areas and mark safe routes.

## Disclaimer

Certain classes such as NPC, Player, and Enemy Combatant were aided by AI. The Player Combatant class was AI-aided but integrated into the rest of the codebase. The combat damage formula is AI-aided since I can't think of a formula.

AI is fine as long as you learn from it and not to rely on it.

The codebase is large and will be refactored in the future.

## Acknowledgements
Basic RPG mechanics. Some contents were inspired by other games.
