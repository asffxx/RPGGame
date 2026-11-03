# RPGGame

A C++17 terminal RPG with turn-based combat, exploration, guild quests, crafting,
and persistent adventures.

**Current release:** v3.6.0

[Download](https://github.com/asffxx/RPGGame/releases/latest) ·
[Development log](DEVLOG.md) · [Game content](content/README.md)

## Features

- 9 races and 13 classes with stat previews and signature combat abilities.
- Five cities and guilds, 10 guild quests, 20 side quests, and 20 repeatable commissions.
- Exploration, travel leads, random encounters, spells, equipment, and hireable companions.
- Nine crafting recipes, eleven materials, a weight-limited backpack, and item selling.
- Manual saves preserve character, inventory, party, quests, discoveries, and story state.
- 15 city residents with branching dialogue, remembered decisions, local jobs, and recipe teaching.
- **New in v3.6.0:** a Story Board with six milestones, unfinished jobs,
  residents' concerns, and remembered choices.

## Play

Download the Windows x64 ZIP or Linux x86_64 archive from
[Releases](https://github.com/asffxx/RPGGame/releases/latest). Extract it and run
the executable from the extracted folder. Keep the included `content/` directory.
Linux requires system glibc. Package checksums are provided with each release.

- Enter the displayed number to choose an action; `0` returns where offered.
- Main categories: **Character**, **Story**, **Local Establishments**, **Actions**, **System**.
- Use **System → Save Game**, then **Continue** next time. Exit does not save.
- Saves use `savegame.dat` in the working directory, not necessarily beside the executable.
- Use **Story → Story Board** to review your progress.
  Completed milestones show their story; future ones offer hints. Viewing costs no time.
- Use **Local Establishments → Talk** to meet city residents.
- Set `NO_COLOR=1` to disable colors; `TERM=dumb` uses plain output.

Save format 4 reads formats 1–3. Back up your save before changing versions or content.

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

Local development checkouts containing `tests/` can run the 35-case suite:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Project layout

- `src/` — application, catalogs, combat, core, entities, gameplay, UI, and world systems.
- `content/` — runtime JSON for characters, items, crafting, narrative, story, and world data.
- `dist/` — v3.6.0 binaries and checksums; see [distribution instructions](dist/README.md).

Public releases use `main`; development continues locally on `inDev`. The original
game is preserved on the [`v1` branch](https://github.com/asffxx/RPGGame/tree/v1).

## Credits and license

Some character classes and combat code were developed with AI assistance.
Some content was inspired by other games. See [LICENSE](LICENSE).
