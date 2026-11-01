# RPGGame

A command-line RPG written in C++.

**Current source release: v3.0.0** · [Development log and version history](DEVLOG.md)

## Description

A text-based RPG with a bit of depth. You create a character, explore a
hand-crafted-yet-random world, fight enemies, manage a party, and grow stronger
through a global scaling progression system.

## Features

- **Race and Class Selection** — 9 races and 13 classes, each with its own stat spread.
- **Selection Previews** — read lore, compare stats, and confirm or go back before creating a character.
- **Save / Continue** — one manual save slot preserves your character, inventory, companions, routes, and time.
- **Class Abilities** — each class has a signature combat action available from level 1, once per encounter.
- **Progression with Global Scaling** — level up and scale stats globally, including enemies.
- **Time System** — day to night, advancing every 4 actions (Morning → Afternoon → Evening → Night).
- **Locations, Events, and Enemies** — an interconnected world with random events and enemy encounters.
- **City Guilds** — five peaceful cities, each with a guild and a two-quest storyline; join all five.
- **Side Quests and Commissions** — 20 one-time side quests and 20 repeatable commissions, with level and guild-story unlocks.
- **Actionable Rumors** — randomized tavern clues unlock city travel leads and offer local quest hints.
- **NPC Generation** — randomly generated NPCs with dialogues, stories, and the ability to hire them at the tavern.
- **Party Management** — up to 4 members, with weekly wages deducted from your gold.
- **Stores** — potions, equipment, food, and drinks.
- **Spells and Enchanting** — learn spells and enchant staffs/weapons.
- **Debuffs and Buffs** — temporary effects that modify combat and stats, including stackable debuffs.
- **Travel System** — random exploration to discover new locations (you might stumble into a high-level area early).
- **Inventory** — equipped and enchanted tags, plus food/drink, potion, and equipment handling.
- **Dictionary** — tracks everything you discover: enemies, locations, weapons, events, and special characters.

## Character Creation

The game opens with a title menu (`Start` / `Continue` / `Exit`). Start asks for three things
in this order:

1. **Race** — pick from the 9 available races, preview lore and bonuses, then Confirm or Back.
2. **Class** — pick from the 13 available classes, preview its playstyle, base stats,
   starting stats with your race, and signature ability, then Confirm or Back.
3. **Name** — type your character's name.

You then start in the Abandoned Cell with your race bonus applied once on top of
your class stats. Back returns to the corresponding selection list.

Main menus are driven by numbers: type the option number and press Enter.
Enter a name during creation; some inventory prompts use `y`/`n`.

## Saving and Continuing

- Select **Save Game** in the main menu to write `savegame.dat` in the current
  working directory. Saving takes no game time and overwrites the single slot.
- Choose **Continue** on the title screen to resume. Run from the same directory
  to find the same save. For example, `./build/rpg` run from the repository root
  stores the save in the repository root, not inside `build/`.
- Saving is **manual**: Exit does not save, and starting another character does
  not overwrite the slot until you select Save Game.
- Saves include stats and progression, currency, spells, inventory and equipped
  items, enchantments and active buffs, companion details, dictionary discoveries,
  current location, discovered/marked routes, sleep, time, and wage timing.
  Guild memberships, discovered jobs, active/completed objectives, commission
  completion counts, accepting guilds, city leads, and tavern rumor history are
  saved too.
- Saving is available at the main menu, outside combat and shop/travel submenus.
  Random-number generator sequences are not saved; future random outcomes can differ.
- Missing, damaged, or unsupported saves show an error and return to the title
  menu. Writes use a temporary file before replacing the previous slot.
- New saves use `RPGGAME 3`. Existing versions 1 and 2 remain readable and start
  with no new side quests/commissions discovered or accepted. Version 2 retains
  guild and rumor progress; version 1 starts those systems empty. The next save
  upgrades the file to version 3. Loading never replays rewards.

## Combat

Combat is turn-based and renders in phases:

- **Your turn** — choose Attack (1), Use Item/Potion (2), Run (3), your class's
  signature ability (4), or Cast Spell (5, if you have learned one).
- **Party turn** — each living party member strikes the enemy.
- **Enemy turn** — the enemy retaliates.

After every full turn the screen refreshes once and waits for Enter, so the whole
exchange is visible on one screen before the next turn starts. The main menu
clears and redraws after each action as well.

### Signature abilities

Every class starts with one signature ability. It consumes a turn and can be
used **once per encounter**; trying an already-used ability does not consume a
turn. A new encounter resets availability. There is no fixed mana cost, but
Blood Rage spends HP and Mana Burst spends half your current mana.

Damage percentages below multiply the corresponding attack stat before enemy
armor. Signature strikes can be dodged except Aimed Shot; they do not roll
critical hits or apply weapon debuffs. Healing cannot revive a dead companion
or exceed maximum HP/mana. Combat displays each ability's effect and usage.

| Class | Ability | Effect |
| --- | --- | --- |
| Warrior | Power Strike | 175% physical damage |
| Knight | Brace | 100% physical damage; halve the next enemy retaliation |
| Berserker | Blood Rage | Spend 10% max HP, stopping at 1 HP; 250% physical damage |
| Assassin | Execute | 250% physical damage if enemy is below half HP; otherwise 125% |
| Rogue | Risky Strike | Equal chance of 300% or 50% physical damage |
| Ranger | Aimed Shot | Undodgeable 150% physical damage |
| Mage | Arcane Bolt | 175% magic damage |
| Sorcerer | Mana Burst | 200% magic damage plus half current mana, spending that mana |
| Scholar | Meditate | Restore 40% max mana |
| Shrewd Diplomat | Distract | Skip the next enemy retaliation |
| Travelling Merchant | Emergency Supplies | Restore 30% max HP |
| Sheltered Noble | Rally | Restore 20% max HP to yourself and each living companion |
| Prisoner | Last Stand | Restore 40% max HP below half HP; otherwise 20% |

Percentage resource costs/restores round down, with healing and Blood Rage's
HP cost having a minimum of 1 where applicable. Brace and Distract expire after
the next retaliation and never modify permanent stats.

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

### Tests

```sh
cmake -B build -S . -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests cover full session round trips and invalid saves, class abilities,
location eligibility, combat menus, all guild quest transitions, rumor discovery,
version-1/2 save migration, all 40 new job definitions, unlock requirements,
repeat cycles, one-time payouts, and interactive guild/board/tavern/travel flows.
If Python 3 is available at configure time, CTest also exercises character reselection, Save/Continue across process
restarts, and load errors. Python is not required to build or play the game.
For multi-configuration Windows builds, pass `-C Release` to CTest after
building with `--config Release`.

## Gameplay Loop

The main menu is grouped into **Character**, **Local Establishments**,
**Actions**, and **System**.

1. **Character** — view stats, open your inventory, manage the party, browse the dictionary, or read the Quest Journal.
2. **Explore** — use the current area's difficulty and terrain for encounters:
   village 20%, town 30%, terrain 60%, dungeon 80%, spell store 50% chance of
   a regular enemy encounter; otherwise a random event occurs. Some events
   have their own named enemies. Peaceful cities offer only noncombat events.
   The Abandoned Cell starts at difficulty 1.
3. **Travel** — venture into the unknown, follow marked safe routes, or follow rumor and quest leads.
4. **Visit establishments** — Stores (villages, towns, cities), Taverns (towns and cities), and Magic Stores (spell stores and cities) offer shopping, hiring, resting, rumors, and spell learning. Each city also has a Guild Hall.
5. **Manage** — keep your inventory, equipment, party, and dictionary in order.
6. **Progress** — defeat enemies for gold and experience, level up, and watch the world scale with you.

Main-menu and travel exploration use the same encounter rules, and companions
join regular and event fights in both. Higher-difficulty locations remain
dangerous even at low player levels. Exploring via a marked safe route avoids
random combat, including combat events. Explicit Quest Actions can still start
quest fights on marked routes. Areas with no eligible enemy pool yield noncombat
events instead.

Every 4 main-menu Explore/Pass Time actions advances the time of day. Each
exploration inside a travel location and each attempted Quest Action advances
it directly. Viewing menus, hearing rumors, joining guilds, accepting/turning in
quests, saving, and marking routes do not advance time.

## Cities, Guilds, and Rumors

The five new cities use the `PeacefulCity` location type. Each has a store,
tavern, magic store, and guild hall. Cities are discoverable by random travel
or by following a tavern clue. The Royal Capital is always referred to by that
title in game.

| City | Guild | Two-quest storyline |
| --- | --- | --- |
| Attania | Ironward Company | The Silent Patrol → Tracks in the Grove |
| Erden | Lantern Scholars | Scattered Pages → The Unquiet Archive |
| Trisselie | Silverthread Consortium | An Unbalanced Ledger → The Last Delivery |
| The Royal Capital | Crownward Order | A Courier's Promise → The Stolen Seal |
| Velmora | Tidebound Fellowship | Beyond the Last Chart → A Treasure with Teeth |

### Following a rumor

1. Enter a **Tavern** in a town or city and choose **Rumors** (option 4).
2. A randomized city clue unlocks a destination under **Travel → Follow a lead**
   (option 3). Unknown-city clues are preferred. City taverns also have their
   own local guild and quest hints, plus clues to other cities.
3. Follow the lead to visit the city. Hearing about it does not grant XP or
   mark it visited; the first physical arrival grants 50 discovery XP once.
4. Choose **Leave** in the location submenu to return to the main menu while
   remaining in that city. Its establishments, including the Guild Hall, appear
   under Local Establishments.

Rumors are free and do not advance time. A tavern avoids repeating its previous
rumor when another is available, even across saves. Leads remain separate from
visited locations and marked safe routes. You can mark a location after arriving;
city leads remain available for later travel. The Quest Journal lists known city
leads and whether you have visited them.

### Completing guild quests

- Visit **Guild Hall**, join for free, then accept the first quest. Membership
  is nonexclusive: you can join all five guilds and have one active quest per guild.
- Accepting a quest makes its objective location available under **Follow a lead**.
  Visiting the guild hall also supplies a return route to its home city.
- At the target location, choose **Quest Action** from the main menu or option 4
  in the travel location submenu. The first quest is a short investigation or
  retrieval scene; the second is a named-enemy confrontation with your party.
- Return to the originating guild to claim the listed silver and XP reward,
  then accept the next quest. Each reward can be claimed only once. Quest items
  are tracked in quest progress rather than occupying sellable inventory slots.
- Running from or losing a quest fight leaves the objective unfinished. A living
  hero can retry; only victory completes it. Ordinary kills or earlier visits do
  not complete these explicit quest objectives.
- A completed objective cannot be repeated for extra combat loot. Normal combat
  rewards are separate from the guild's turn-in reward. The second turn-in
  completes that guild's short storyline.

Use **Quest Journal** to review your objectives and return instructions. Save
manually from the main menu to keep memberships, progress, rewards, and leads.

### Side quests and commissions

Every Guild Hall offers **Side Quests (3)** and **Commissions (4)**, even after
its own storyline is finished. There are **40 additional jobs**: 20 side quests
and 20 commissions, alongside the existing ten guild storyline quests. Main
quests have a separate module and journal section reserved for future content.

- Boards show relevant work and explain locked requirements. Viewing a board
  discovers its jobs; accepting one checks all its requirements again.
- **General** jobs appear at every guild and require no guild membership. They
  can be accepted at one guild and turned in at another.
- **Guild-specific** jobs appear only at their offering guild, require membership,
  and must be turned in there. Each guild offers two side quests and two commissions.
- Meet the stated character level and any guild-quest prerequisite. A prerequisite
  counts only after **turning in its reward**, not just finishing its objective.
- Accept a job, follow its travel lead, choose its **Quest Action** at the target,
  then return to a guild board to turn it in. The journal groups work by category.
- Several different jobs may be active together, but the same job cannot be
  accepted twice—even at different guilds. Each action completes only the selected
  objective when jobs share a destination or enemy.
- Side quests remain completed permanently. Commissions become available again
  after turn-in; explicitly accept a new run and redo its objective to earn another
  reward. The journal retains the number of completed runs.
- Retreat/defeat does not finish a job or pay rewards. Completed objectives cannot
  be replayed for loot while awaiting turn-in. Quest actions use one time period;
  board browsing, acceptance, and turn-in are free of time costs.

The following tables list the current content. Guild abbreviations mean Ironward
Company, Lantern Scholars, Silverthread Consortium, Crownward Order, and Tidebound
Fellowship. A dash means no guild-story prerequisite; all other access rules still apply.

#### 20 side quests (one-time)

| Quest | Board | Level | Required guild quest turn-in |
| --- | --- | --- | --- |
| A Letter Never Sent | General | 1 | — |
| The Herbalist's Keepsake | General | 2 | — |
| The Broken Waystone | General | 3 | — |
| A Lantern for the Lost | General | 3 | — |
| The Oasis Promise | General | 2 | — |
| A Warden's Last Watch | Ironward | 2 | The Silent Patrol |
| The Unwritten Chapter | Lantern | 3 | The Unquiet Archive |
| The Honest Counterfeit | Silverthread | 3 | An Unbalanced Ledger |
| A Seal Without a Name | Crownward | 4 | The Stolen Seal |
| The Navigator's Debt | Tidebound | 3 | Beyond the Last Chart |
| The Winter Song | General | 3 | — |
| The Hollow Tree | General | 2 | — |
| The Empty Granary | General | 1 | — |
| A Memory in Glass | General | 4 | — |
| The Ruined Oath | General | 4 | — |
| The Warning Bell | Ironward | 1 | — |
| The Apprentice's First Error | Lantern | 1 | — |
| A Very Small Debt | Silverthread | 2 | — |
| The Fallen Banner | Crownward | 3 | A Courier's Promise |
| The Last Beacon | Tidebound | 4 | A Treasure with Teeth |

#### 20 commissions (repeatable)

| Commission | Board | Level | Required guild quest turn-in |
| --- | --- | --- | --- |
| Roadside Supplies | General | 1 | — |
| Forest Samples | General | 2 | — |
| Rat-Clearing Contract | General | 1 | — |
| Swamp Survey | General | 3 | — |
| Wolf Bounty | General | 2 | — |
| Ironward Patrol | Ironward | 2 | Tracks in the Grove |
| Archive Maintenance | Lantern | 2 | Scattered Pages |
| Cargo Recovery | Silverthread | 3 | The Last Delivery |
| Crown Road Watch | Crownward | 3 | A Courier's Promise |
| Cavern Salvage | Tidebound | 4 | A Treasure with Teeth |
| Milepost Inspection | General | 1 | — |
| Oasis Well Check | General | 2 | — |
| Viper Removal | General | 3 | — |
| Frost Readings | General | 3 | — |
| Belfry Clearance | General | 4 | — |
| Watchpost Rations | Ironward | 1 | — |
| Botanical Ink | Lantern | 1 | — |
| Caravan Audit | Silverthread | 2 | — |
| Dispatch Cache Check | Crownward | 2 | — |
| Tidal Marker Survey | Tidebound | 3 | — |

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

### Guilds, rumors, and persistence

- `src/data/guild.hpp/.cpp`: guild IDs, cities, introductions, and links to their
  ordered storyline definitions.
- `src/data/rumor.hpp/.cpp`: stable rumor IDs, city-local hints, and city clues.
- `src/systems/guild_state.hpp`: persistent guild progress, city leads, and
  per-tavern rumor history, owned by `GameSession`.
- `src/systems/guild.cpp`: membership, guild halls, board entry points, and
  validation of guild/rumor state.
- `src/systems/rumor.cpp`: eligible rumor pools, random selection, repeat
  avoidance, and unlocking city leads.
- `src/world/travel.cpp`: lead selection and once-only arrival/discovery credit.
- `src/systems/save.cpp`: version-3 serialization, version-1/2 migration, and
  validation of guild/quest/rumor state.

### Shared quest handling (`src/world/`)

- `guild_quest.hpp/.cpp`: existing guild storyline content, prerequisite checks,
  and compatibility adapters for guild-stage progress.
- `quest.hpp/.cpp`: main-story catalog (currently empty).
- `sidequests.hpp/.cpp`: the 20 one-time side-quest definitions.
- `comissions.hpp/.cpp`: the 20 repeatable commission definitions. The filename
  deliberately uses the requested spelling `comissions`; menus use `Commissions`.
- `quest_types.hpp`: common quest definitions, categories, and persistent job
  progress (`GameSession::quests`).
- `quest_system.hpp/.cpp`: combined catalog, eligibility, acceptance, objective
  execution, rewards, boards, journal, travel leads, and saved-state validation.

All categories use the same lifecycle and combat execution. Guild storylines
retain their existing stage-based save representation behind that shared API.

### Class skills and store spells

- `src/data/class_skill.hpp` / `class_skill.cpp`: `ClassSkillDatabase` holds
  class-skill names, descriptions, effect kinds, damage multipliers, costs,
  healing amounts, thresholds, and dodge/defense settings. Edit class-skill
  balance values here and keep their descriptions in sync.
- `src/combat/abilities.hpp` / `abilities.cpp`: execute those effects and track
  once-per-encounter usage and temporary retaliation effects. Character previews
  and combat menus read the same class-skill catalog.
- `src/data/spell.hpp` / `spell.cpp`: the separate `SpellDatabase` for store-bought
  spells, including fixed damage, mana costs, prices, and enchantment settings.
  Spell casting is handled by `CombatScreen::handleCastSpell()`.

Class skills are the intended foundation for future leveling, evolution, and
sister-skill branches. Those progression mechanics are not implemented yet;
current class skills scale with character stats. Store-bought spells have no
spell-level progression and retain their catalog damage when cast.

The codebase was originally a single `main.cpp` and is now split into modules
under `src/`:

```
src/
  app/        main menu loop and all text output
  combat/     combatants, damage calculation, fight loop
  core/       stats, economy, time, UI primitives
  data/       static catalogs (races, classes, skills, items, enemies, events, guilds, rumors)
  entities/   Player, Enemy, NPC and their generators
  systems/    inventory, events, party, dictionary, guilds, rumors, saves
  ui/         menu rendering and the player stat panel
  world/      travel, establishments, quest categories and shared quest handling
```

## Disclaimer

Certain classes such as `NPC`, `Player`, and `EnemyCombatant` were aided by AI. The `PlayerCombatant` class was AI-aided but integrated into the rest of the codebase. The combat damage formula is AI-aided since I can't think of a formula.

## Acknowledgements

Basic RPG mechanics. Some contents were inspired by other games.
