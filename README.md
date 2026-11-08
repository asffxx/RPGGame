# RPGGame

A C++17 terminal RPG with turn-based combat, exploration, guild quests, crafting,
and persistent adventures.

**Version:** v4.0.1.

> The game will be continued in Godot in a separate repo. See the
> [development log](DEVLOG.md).

[Download](https://github.com/asffxx/RPGGame/releases/latest) ·
[Development log](DEVLOG.md) · [Game content](content/README.md)

## Features

- 9 races and 13 classes with stat previews and signature combat abilities.
- Five cities and guilds, 10 guild quests, 25 side quests, and 20 repeatable commissions.
- Exploration, travel leads, random encounters, spells, equipment, and hireable companions.
- Nine crafting recipes, eleven materials, a weight-limited backpack, and item selling.
- Manual saves preserve character, inventory, party, quests, discoveries, and story state.
- 15 city residents with branching dialogue, remembered decisions, local jobs, and recipe teaching.
- A Story Board with six milestones, unfinished jobs,
  residents' concerns, and remembered choices.
- **New in v4.0.1:** richer ASCII panels, numbered confirmations, paged inventory,
  shop, upgrade, and repair lists, and category navigation that stays where you left it.
  Combat rewards share one victory panel; numbers use compact formatting.
  Fixed six event encounters that referenced missing enemies.
- **New in v4.0.0:** debuffs apply through resist bar meters instead of
  chance rolls. Each combatant tracks a 0–100 fill per debuff; landed hits add
  20 points, slowed to half armor fraction resistance (capped 50%); a full bar
  applies the debuff and empties. Bars decay 5 points per round and are combat-only.
  No save-format change.
- **New in v3.8.0:** status effects share one duration and stacking model;
  equipment can be upgraded to +3 and repaired; crafting, upgrades, and repairs
  charge copper fees. Defense and crit stats use their authored units.
- **New in v3.7.0:** guild reputation changes resident reactions, quest offers,
  shop stock, and prices. Standing appears in the dashboard and Story Board.

## Play

Download the Windows x64 ZIP or Linux x86_64 archive from
[Releases](https://github.com/asffxx/RPGGame/releases/latest). Extract it and run
the executable from the extracted folder. Keep the included `content/` directory.
Linux requires system glibc. Package checksums are provided with each release.

- Enter the displayed number to choose an action; `0` returns where offered.
- Main categories: **Actions**, **Character**, **Local Establishments**, **Story**, **System**.
- After an action, you return to its category. Choose **0. Back** for the overview.
- Paged lists show explicit Back and page-navigation choices.
- Use **System → Save Game**, then **Continue** next time. Exit does not save.
- Saves use `savegame.dat` in the working directory, not necessarily beside the executable.
- Use **Story → Story Board** to review your progress.
  Completed milestones show their story; future ones offer hints. Viewing costs no time.
- Use **Local Establishments → Talk** to meet city residents.
- Helpful resident decisions give +3 standing; dismissive decisions give -4.
  Guild storyline rewards give +3. Friendly starts at 6, hostile at -6; no decay.
- Friendly cities offer five new local jobs and 10% shop discounts. Hostile
  cities refuse new jobs and charge 20% more; active jobs can still be turned in.
- Use **Local Establishments → Store → Repair Equipment** to restore worn gear.
  At the forge, choose **Upgrade equipment** to spend materials and copper on +1 to +3.
- Set `NO_COLOR=1` to disable colors; `TERM=dumb` uses plain output.

Save format 5 reads formats 1–4. Back up your save before changing versions or content.

## Build from source

Requires CMake 3.16+ and a C++17 compiler.

```sh
cmake -S . -B build
cmake --build build --config Release
./build/rpg
```

On Windows, run `build\Release\rpg.exe` with Visual Studio, or `build\rpg.exe`
with a single-config generator such as MinGW Makefiles.

Run from the repository root, or set `RPG_CONTENT_DIR` to the absolute path of
`content/`. Lookup order: environment override, working-directory `content/`,
then the build-time source directory. An invalid explicit override fails startup.

## Project layout

- `src/` — application, catalogs, combat, core, entities, gameplay, UI, and world systems.
- `content/` — runtime JSON for characters, items, crafting, narrative, story, and world data.
- Release builds are published on the [Releases page](https://github.com/asffxx/RPGGame/releases/latest); `dist/` is local build output and not in the repo.

The original game is preserved on the [`v1` branch](https://github.com/asffxx/RPGGame/tree/v1).

## Credits and license

Some character classes and combat code were developed with AI assistance.
Some content was inspired by other games. See [LICENSE](LICENSE).
