# Development log

RPGGame's development history, newest milestone first. Unreleased work is marked
separately from published releases.

## v3.7.0 — Reputation and faction standing

**Date:** 2026-11-05 — Windows x64 and Linux x86_64 release builds.

- Added five guild standings, derived city standing, a -100..100 clamp, and
  friendly/hostile thresholds at 6/-6. No decay or save-format change.
- All 15 residents have authored tier reactions, +3 helpful decisions, and a
  once-only -4 dismissive decision. Guild storyline claims grant +3 standing.
- Hostile halls refuse new work while allowing active jobs to be turned in.
  Five friendly-only side jobs remain visible with their lock reasons.
- City shops filter stock and apply a 10% friendly discount or 20% hostile
  markup. Displayed and charged prices match; villages and towns stay neutral.
- Added current-city dashboard standing and all five standings to the Story Board.
- Rules and stock live in `content/narrative/reputation.json`.
- Seven new test groups bring coverage to 42 cases; formats 1–4 remain readable.
- Linux and Windows Release builds passed all 42 tests, with Windows execution
  verified through Wine. Packaged executables passed version, startup, save/load,
  dependency, and checksum checks with matching runtime content.

## v3.6.0 — Story board

**Date:** 2026-11-04 — Windows x64 and Linux x86_64 release builds.

- Added a Story Board under a new main-menu Story category. It shows Act One,
  six sequential beats, open threads, and every remembered dialogue choice.
- Completed beats show their full text; future beats show hints. Already-met
  requirements advance together in order when the board opens, without changing
  the save or advancing game time.
- Story text lives in `content/story/acts.json`; UI code carries no act or beat
  strings. Beat state is derived from flags, quests, recipes, guild membership,
  travel, and standing, so save format stays 4.
- Extracted dialogue condition parsing and evaluation into a shared engine.
  Added a `count` condition for level, completed quests, known recipes, joined
  guilds, discovered cities, recorded choices, and highest standing.
- Open threads list active jobs, including current guild storyline quests, plus
  unresolved residents in cities the player has reached.
- The final beat is an unexplained omen after seeing all five cities. No collapse
  cause, main quest, or responsible race was authored.
- Added six story test groups; all 35 CTest cases pass and formats 1–4 still load.
- Reused rule-validation tables, reserved story-view storage, and skipped checks
  for locked beats and redundant completion scans.
- Packaged Linux and Windows binaries with matching content and SHA-256 checksums.
- Both extracted packages passed Continue, Story Board, and Exit checks; the
  Windows executable was tested under Wine.

## v3.5.0 — Peaceful-city dialogue

**Date:** 2026-11-03 — Windows x64 and Linux x86_64 release builds.

- Added 15 authored, non-recruitable residents: three in each city, accessible
  through Local Establishments → Talk. Each has local lore, a branching decision,
  and remembered responses. Guild stories remain independent of the main quest.
- Added JSON dialogue graphs with strict validation of IDs, references, effects,
  conditions, node destinations, and reachability. Missing/invalid dialogue data
  stops startup with a file-specific error.
- Added conditions for flags, guild standing, items, quests, race, class, recipes,
  and recorded choices; unavailable menu options explain their requirements.
- Added atomic effects for flags, item gifts, quest acceptance, standing, recipe
  teaching, and choice recording. Failed rewards leave all affected state intact.
- Five craftspeople teach existing recipes. Five guild representatives introduce
  existing local jobs. A Letter Never Sent now includes an optional bread reward.
- Save format remains 4; decisions and responses survive reload. Added six
  CTest groups; all 29 cases pass. Dialogue menus handle Back, Leave, and EOF.
- The collapse, its cause, and the main quest remain deliberately unwritten.
- Packaged both platforms with matching runtime content and SHA-256 checksums.
  Added the explicit algorithm header needed by the LLVM-MinGW Windows build.

## v3.4.2 — Content JSON migration and catalog infrastructure relocation

**Date:** 2026-11-03 — content migration and rebuilt release artifacts.

- Migrated all remaining authored game content to `content/`, making JSON the
  sole source of truth for narrative and NPC data:
  - `content/narrative/guilds.json` — 5 guilds, 10 embedded storyline quests.
  - `content/narrative/rumors.json` — 15 rumors (5 road leads, 10 tavern).
  - `content/narrative/sidequests.json` — 20 side quests.
  - `content/narrative/commissions.json` — 20 repeatable commissions.
  - `content/narrative/mainquests.json` — empty main-quest catalog (reserved).
  - `content/characters/npc_pools.json` — name pools, special characters,
    backstories, dialogue lines, starter items.
- Updated all catalog C++ loaders (`guild.cpp`, `rumor.cpp`, `sidequests.cpp`,
  `commissions.cpp`, `quest.cpp`, `npc.cpp`, `establishments.cpp`) to read from
  `content/` via `content::catalog<T>()`; all hardcoded vectors removed.
- Relocated `src/core/ids.cpp/.hpp` → `src/catalogs/content_ids.cpp/.hpp` and
  `src/core/paths.cpp/.hpp` → `src/catalogs/content_paths.cpp/.hpp`. Content
  infrastructure now lives beside the catalog layer it serves. `src/core/` is
  engine-only (economy, stats, timesystem, ui).
- Updated content loaders, gameplay callers, and validation code to include the new paths
  directly. No forwarding headers left behind.
- Updated `CMakeLists.txt`: removed `src/core/ids.cpp` and `src/core/paths.cpp`;
  added `src/catalogs/content_ids.cpp` and `src/catalogs/content_paths.cpp`.
- Bumped the project version to 3.4.2 in `CMakeLists.txt` and
  `src/core/version.hpp`, and synced the README release line.
- Rebuilt the v3.4.2 release artifacts for Linux x86_64
  and Windows x64 (MinGW-w64). Updated `dist/README.md` and `SHA256SUMS`;
  both binaries launch and report v3.4.2.
- Corrected stale documentation in `README.md` and `content/README.md`:
  `RPG_CONTENT_DIR` (not `RPG_DATA_DIR`), current `content/` paths, the 23-test
  suite, `src/catalogs/` and `src/gameplay/` layout, save format 4, and
  JSON-sourced catalog ids.

## v3.4.1 — Project structure refactor

**Date:** 2026-11-01

- Renamed `data/` to `content/` and reorganized JSON files into subdirectories
  (`crafting/`, `narrative/`, `characters/`, `world/`, `items/`).
- Renamed `src/systems/` to `src/gameplay/` and `src/content/` to `src/catalogs/`.
- Renamed `src/ui/display.cpp` to `src/app/display.cpp`.
- Moved `src/gameplay/datafile.*` to `src/gameplay/datafile.*`.
- Updated all C++ include paths, CMake configurations, and JSON loader paths.
- Updated path resolution to use `content/` instead of `data/`.
- Verified build and 23/23 tests pass; verified catalog snapshot consistency.

## v3.4.0 — Crafting, material drops, backpack, and selling

**Date:** 2026-11-01 — crafting and inventory improvements.

- Added eleven materials and nine recipes in `data/materials.json` and
  `data/recipes.json`, with validated item, enemy, quest, guild, and station references.
- Added city/guild-hall forge, alchemy, and kitchen menus. Crafts are atomic and
  account for consumed ingredient weight before checking the output capacity.
- Recipes unlock through starter availability, guild membership, quest completion,
  or an exploration flag. Known recipes persist; NPC instruction follows in Phase 4.
- Added independently rolled enemy materials to all four victory reward paths.
  Two commissions supply materials; insufficient space leaves their rewards unclaimed.
- Added a 50-weight backpack. Above 37.5, basic attack stats lose 10% and dodge
  loses 10 percentage points. Full packs block purchases and material pickups.
  Legacy overweight inventories remain loadable.
- Stores buy items at half their shop price; materials use their JSON sell value.
  Selling or crafting cannot remove the final equipped copy.
- Save format remains 4, including material ids and known recipes. Runtime data
  files are now required alongside future distributions.
- Added five crafting test groups: catalog validation, atomic transactions,
  backpack penalties, drops/rewards/save roundtrip, and interactive menu flows.
- Verified 23/23 CTest cases, plus a real-binary Continue → craft → save → reload
  → sell → save flow and the missing-data startup error.

## v3.3.0 — Content ids, JSON reader, save format 4

**Date:** 2026-11-01

- Added stable snake_case ids to every catalog record: equipment, potions, food,
  locations, enemies, classes, races, spells, guilds, and quests. Lookups by id
  and by table position both work; duplicate display names get `_2` suffixes.
- Added `src/gameplay/datafile.cpp`, a strict reader for the JSON subset shared
  with the future pixel engine. Comments, trailing commas, duplicate keys, lone
  surrogates, and non-finite numbers are rejected. Errors name file, line, and
  column.
- Added `data/README.md` describing the format, id rules, cross-reference rules,
  and the files that each later phase will add.
- Save format 4 adds story flags, recorded choices, known recipes, and faction
  standing, plus stable item ids. Versions 1, 2, and 3 stay readable and gain
  item ids on load. Unknown or mismatched ids are hard errors.
- Inventory items now carry their catalog id, and `PlayerInventory` resolves
  ids to catalog indices.
- Test count rises from 15 to 18: `catalog_ids`, `json_reader`, and `story_save`.
- Source release only. Prebuilt binaries remain at v3.2.1 until the release
  step rebuilds and checksums them.

## v3.2.1 — Standalone Windows and Linux builds

**Date:** 2026-11-01

- Added prebuilt **Windows 10/11 x64** and **Linux x86_64** executables under
  `dist/`, with a distribution README and SHA-256 checksums.
- Windows build is a static MinGW-w64 UCRT console executable; no DLLs required.
- Linux build is statically linked and runs without a separate C++ runtime.
- Verified both binaries launch and show the v3.2.1 title screen. Windows checks
  were run under Wine; full Linux tests remain green.
- No gameplay or save-format changes. Patch release: 3.2.0 to 3.2.1.

## v3.2.0 — Shorter, category-based main menu

**Date:** 2026-11-01

- Replaced the flat main menu with **Character**, **Local Establishments**,
  **Actions**, and **System** categories.
- The top level now shows four choices (three outside settlements) instead of up
  to eleven actions. Selecting a category opens only its actions; `0. Back`
  returns without running anything.
- Establishments remain location-aware and appear only where stores, taverns,
  magic stores, or guild halls exist.
- Kept the character/status dashboard and numbered controls. Actions still refresh
  the menu after returning.
- Updated gameplay Basics text, README usage instructions, and CLI flows for the
  new navigation. All 15 tests pass.

Backward-compatible interface change: game version advances from 3.1.0 to
3.2.0. Save format remains 3.

## v3.1.0 — Terminal interface refresh

**Date:** 2026-11-01

- Retained portable numbered controls while introducing consistent ASCII panels,
  terminal-width wrapping, and subtle heading and border colors.
- Added a versioned title screen and an at-a-glance main-menu display with
  character, class, level, HP/mana bars, location, time, and currency.
- Applied the shared panel renderer to combat, travel, guild boards, the journal,
  and other existing bordered menus.
- Added plain-text output for redirected streams and `TERM=dumb`, plus `NO_COLOR`
  support. Screen clearing is skipped when output is redirected or the terminal
  is marked dumb.
- Removed the Python dependency from CLI verification. Equivalent creation,
  save/restart, and malformed-save checks now run through CMake.
- Added C++ panel-wrapping, UTF-8 boundary, and resource-bar checks. All 15 tests
  pass on Linux.
- Updated usage instructions and linked the original game on the `v1` branch.

This is a backward-compatible feature release: game version advances from
3.0.0 to 3.1.0. The save format stays at 3 and still reads formats 1 and 2.

## v3.0.0 — Persistent adventures and shared quests

**Date:** 2026-11-01

This milestone turns individual play sessions into persistent adventures, with
guild storylines, discoverable jobs, and more distinct character choices.

### Save and continue

- Added a manual save slot and Continue from the title screen.
- Saves preserve the character, inventory, companions, travel progress, time,
  guild memberships, rumor history, and quest progress.
- Save format 3 records discovered jobs, active objectives, accepting guilds,
  and commission completion counts. Formats 1 and 2 remain readable.
- Added save validation and migration without replaying rewards.

### Characters and combat

- Added race/class lore and stat previews, confirmation, and Back navigation
  before character creation. Fixed screen clearing when returning to race selection.
- Added a signature combat ability for each of the 13 classes, available from
  level 1 and usable once per encounter.
- Made enemy encounters location-aware.

### Cities, guilds, and discovery

- Added five peaceful cities: Attania, Erden, Trisselie, Ariandel, The Royal Capital,
  and Velmora.
- Added five guilds with two storyline quests each. A character can join all five.
- Added randomized tavern rumors, city travel leads, and quest hints.
- Kept discovering a lead distinct from physically visiting its destination.

### Forty new jobs

- Added **20 one-time side quests** and **20 repeatable commissions**, alongside
  the **10 guild storyline quests**: **50 playable quests in total**.
- Added level, guild-membership, and specific guild-quest prerequisites.
  Guild prerequisites count after reward turn-in, not objective completion alone.
- General jobs are available at every guild and can be turned in at any hall.
  Guild-specific jobs require membership and their originating hall.
- Added Side Quests and Commissions boards, including after a guild's storyline ends.
- Integrated objectives with combat, travel leads, and a categorized Quest Journal.
- Commissions require explicit reacceptance and a fresh objective each run.
  Completion counts persist; side quests cannot be repeated.
- Prevented duplicate acceptance, repeated payouts, and multiple quest completions
  from one selected objective action.

### Internal structure and verification

- Added shared quest definitions, progress, and handling in `src/world/`.
- Split content into `guild_quest.*`, `quest.*`, `sidequests.*`, and `commissions.*`.
  The main-story catalog in `quest.*` is reserved for future content and is empty.
- Added persistent `GameSession` state and reusable combat ability handling.
- Expanded CMake/CTest coverage to 14 tests, including save migration, unlock
  rules, commission cycles, guild boards, combat menus, and CLI flows.
- Verified the build and all 14 tests on Linux; updated gameplay documentation.

## v2.0.0 — Refactoring milestone (retrospective)

**Period:** 2026-07-09 through 2026-10-30

The second milestone reorganized the existing game into maintainable modules
and improved build portability. This is a retrospective milestone label, not
an assertion that a `v2.0.0` tag was published at the time.

- [Initial refactor (`be6d543`)](https://github.com/asffxx/RPGGame/commit/be6d543)
  split the large `src/main.cpp` into core, data, entities, combat, systems, UI,
  application, and world modules.
- [Directory restructuring (`db9e14d`)](https://github.com/asffxx/RPGGame/commit/db9e14d)
  refined the source layout.
- [Display separation (`17e6ada`)](https://github.com/asffxx/RPGGame/commit/17e6ada)
  moved gameplay output into centralized display helpers.
- [Character-creation refactor (`e34675d`)](https://github.com/asffxx/RPGGame/commit/e34675d)
  organized creation and display code; subsequent work renamed NPC/enemy entities.
- [Portable screen clearing (`08d3538`)](https://github.com/asffxx/RPGGame/commit/08d3538)
  improved cross-platform terminal behavior.
- [Build/tooling update (`0125cdf`)](https://github.com/asffxx/RPGGame/commit/0125cdf)
  added the current CMake setup, tracked clangd configuration, and refreshed README.

## v1.1.0 — Early interface and travel fixes

**Date:** 2026-01-14

The history explicitly names this version in
[`0e2c157` — Update v1.1.0](https://github.com/asffxx/RPGGame/commit/0e2c157).

- Added consistent screen clearing around travel and combat.
- Shortened the startup introduction.
- Fixed marked-location travel failing to update the player's current location
  and location type in the main menu.

## v1.0.0 — Initial project (retrospective)

**Date:** 2026-01-14

The original v1 game is preserved on the
[`v1` branch](https://github.com/asffxx/RPGGame/tree/v1), whose current tip is
[`19b50cb`](https://github.com/asffxx/RPGGame/commit/19b50cb). That branch includes
the early v1.1.0 changes and predates the modular refactor; `main` carries current development.

- [Initial commit (`9c33a81`)](https://github.com/asffxx/RPGGame/commit/9c33a81)
  created README, license, and Git attributes.
- [Game commit (`b231a88`)](https://github.com/asffxx/RPGGame/commit/b231a88)
  introduced the playable C++ game in a single large `src/main.cpp`.
- Established the original character creation, combat, exploration, inventory,
  progression, party, and establishment systems that later versions build on.

The initial-project label groups these two commits; the initial commit alone
did not contain the game source.

## Versioning

`v3.0.0` establishes the current release baseline using `vMAJOR.MINOR.PATCH`
tags. The v1/v2 milestone labels above preserve the project's development eras;
they do not retroactively create release tags.

Future releases follow semantic versioning: incompatible supported-interface or
save-compatibility changes increment MAJOR, backward-compatible features increment
MINOR, and backward-compatible fixes increment PATCH. Save-format numbers are
separate from game release versions. Add each release entry at the top of this log.
