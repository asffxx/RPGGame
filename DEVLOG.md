# Development log

RPGGame's development history, newest milestone first. Entries describe shipped
source changes and link to the commits behind earlier milestones.

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
- Removed `tests/cli_test.py` and Python discovery from CMake. Moved equivalent
  creation, save/restart, and malformed-save CLI checks to `tests/cli_test.cmake`.
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

- Added five peaceful cities: Attania, Erden, Trisselie, The Royal Capital,
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
- Split content into `guild_quest.*`, `quest.*`, `sidequests.*`, and `comissions.*`.
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
