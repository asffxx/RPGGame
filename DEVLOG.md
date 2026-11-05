# Development log

RPGGame's development history, newest milestone first. Add each release entry at the top.

## v4.0.0 — Debuff resistances and feature freeze

**Date:** 2026-11-06 — Windows x64 and Linux x86_64 release builds.

- Debuffs now apply through resist-bar meters instead of chance rolls. Each
  combatant keeps a hidden 0–100 fill per debuff; hits add 20 points, slowed
  by armor resistance; a full bar applies the debuff and empties.
- Armor resistance is half the armor fraction, capped at 50% (player/NPC from
  equipped armor; enemy from armor stat). Meters decay 5 points per round,
  clear at combat start and end, and are never saved. Save format stays 5.
- Removed the old `effectChance` and `debuffChance` catalog fields.
- Three new Phase 8 test groups; 50/50 on both fresh platform builds, zero
  compiler warnings, checksums verified.
- Repo is frozen for new features at v4.0.0. The pixel game continues in a
  separate repo.

## v3.8.0 — Gap mechanics

**Date:** 2026-11-05 — Windows x64 and Linux x86_64 release builds.

- One duration/stacking model for all debuffs: per-stack damage and drain,
  refresh on reapply, maximum-stack amplifiers, percentage-point armor
  reductions. All equipment, spell, and enemy references audited.
- Equipment upgrades to +3 using materials and copper; added durability and
  repair. Broken gear is unequipped after combat.
- Defense and magic defense are percentage points; crit damage is a multiplier
  bonus. Upgrades follow the +15% base-stat curve; spell armor buffs are
  temporary percentage points removed at combat end.
- Economy sinks: per-recipe, upgrade, and repair fees. Crafting stays atomic
  when a fee cannot be paid.
- Save format 5 stores debuff duration/stacks and per-item upgrade level and
  durability. Formats 1–4 still load; legacy debuff names migrate.
- Balance rules are documented, every row tied to a test.
- Five new Phase 7 test groups; 47/47 with no compiler warnings.

## v3.7.0 — Reputation and faction standing

**Date:** 2026-11-05 — Windows x64 and Linux x86_64 release builds.

- Five guild standings, derived city standing, -100..100 clamp, thresholds at
  6/-6 (friendly/hostile). No decay or save-format change.
- 15 residents with authored tier reactions: +3 helpful, once-only -4
  dismissive, +3 guild claims. Hostile halls refuse new work; active jobs stay
  turnable.
- Friendly shops: 10% discount, five new jobs. Hostile: 20% markup. Small
  settlements stay neutral.
- Standing shown on the dashboard and Story Board. Rules in
  `content/narrative/reputation.json`.
- Seven new test groups; 42/42, formats 1–4 readable, Windows verified in Wine.

## v3.6.0 — Story board

**Date:** 2026-11-04 — Windows x64 and Linux x86_64 release builds.

- New Story menu: Act One with six sequential beats, open threads, and
  remembered choices. Beat state is computed from existing flags — save
  format stays 4.
- Completed beats show full text; future ones show hints. Requirements advance
  in order without changing the save or game time.
- Story text in `content/story/acts.json`; UI code carries no beat strings.
  Final beat is an unexplained omen; no collapse cause is authored.
- Dialogue conditions extracted into a shared engine with a new `count`
  condition (level, quests, recipes, guilds, cities, choices, standing).
- Six new test groups; 35/35, formats 1–4 readable.

## v3.5.0 — Peaceful-city dialogue

**Date:** 2026-11-03 — Windows x64 and Linux x86_64 release builds.

- 15 authored, non-recruitable residents (three per city) with lore,
  branching decisions, and remembered responses.
- JSON dialogue graphs with strict validation; bad data stops startup with a
  named file error.
- Conditions cover flags, standing, items, quests, race, class, recipes, and
  choices; unavailable options explain their requirements.
- Atomic effects: flags, item gifts, quest acceptance, standing, recipe
  teaching, choice recording. Failed rewards leave state intact.
- Five craftspeople teach recipes; five guild reps introduce local jobs.
- Save format stays 4. Six new test groups; 29/29. The main quest and its
  cause remain deliberately unwritten. Fixed the missing algorithm header in
  the Windows build.

## v3.4.2 — Content JSON migration

**Date:** 2026-11-03 — content migration and rebuilt release artifacts.

- All remaining content moved to `content/`: guilds, rumors, side quests,
  commissions, main quest (reserved, empty), NPC pools.
- All catalog loaders read from `content/` via `content::catalog<T>()`;
  hardcoded vectors removed.
- `src/core/ids.*` and `paths.*` relocated to `src/catalogs/`; `src/core/` is
  engine-only.
- Version bumped to 3.4.2; release artifacts rebuilt and checksummed; stale
  docs corrected (`RPG_CONTENT_DIR`, paths, save format 4).

## v3.4.1 — Project structure refactor

**Date:** 2026-11-01

- Renamed `data/` → `content/` (subfolders: crafting, narrative, characters,
  world, items), `src/systems/` → `src/gameplay/`, `src/content/` →
  `src/catalogs/`, `src/ui/display.cpp` → `src/app/display.cpp`.
- Updated all include paths, CMake, and loader paths; content lookups resolve
  via `content/`.
- 23/23 tests pass; catalog snapshot consistency verified.

## v3.4.0 — Crafting, drops, backpack, and selling

**Date:** 2026-11-01 — crafting and inventory improvements.

- Eleven materials, nine recipes with validated cross-references; city/guild
  forge, alchemy, and kitchen menus. Crafting is atomic; recipes unlock by
  play, known recipes persist.
- All four victory paths roll for materials; two commissions supply them.
  Insufficient space leaves rewards unclaimed.
- 50-weight backpack: over 37.5 costs 10% attack and 10 points of dodge; full
  packs block purchases and pickups. Legacy overweight saves stay loadable.
- Selling pays half shop price (materials use their JSON sell value); the last
  equipped copy can never leave the character.
- Save format stays 4. Five crafting test groups; 23/23 plus a real binary
  craft/save/reload/sell flow and the missing-data startup error.

## v3.3.0 — Content ids, JSON reader, save format 4

**Date:** 2026-11-01

- Stable snake_case ids on every catalog record; id and position lookups both
  work; duplicate names get `_2` suffixes.
- Strict JSON-subset reader: errors name file, line, column; rejects comments,
  trailing commas, duplicate keys, non-finite numbers.
- Save format 4 adds story flags, choices, known recipes, standing, and item
  ids. Formats 1–3 readable; unknown or mismatched ids are hard errors.
- Inventory items carry catalog ids resolved by `PlayerInventory`.
- 18 tests (from 15). Source-only release.

## v3.2.1 — Standalone Windows and Linux builds

**Date:** 2026-11-01

- Prebuilt Windows x64 (static MinGW UCRT console, no DLLs) and Linux x86_64
  (statically linked) executables, with README and checksums.
- Both launch and show v3.2.1; Windows checked under Wine; full Linux tests
  green. No gameplay or save-format changes.

## v3.2.0 — Shorter, category-based main menu

**Date:** 2026-11-01

- Flat menu replaced by **Character**, **Local Establishments**, **Actions**,
  **System** categories; location-aware establishments only.
- Numbered controls and dashboard kept. 15/15 tests. Interface change
  3.1.0 → 3.2.0; save format stays 3.

## v3.1.0 — Terminal interface refresh

**Date:** 2026-11-01

- Consistent ASCII panels, width-aware wrapping, heading/border colors,
  versioned title screen, stats dashboard with HP/mana bars.
- Shared panel renderer applied to combat, travel, guild boards, and journal.
- Plain output for `TERM=dumb` and redirects; `NO_COLOR` support.
- CLI verification ported from Python to CMake; panel/UTF-8/resource-bar
  checks added. 15/15 tests. Save format stays 3.

## v3.0.0 — Persistent adventures and shared quests

**Date:** 2026-11-01

- Manual save slot and Continue; saves cover character, inventory, party,
  travel, time, guilds, rumors, and quest progress. Save format 3 records
  jobs, objectives, accepting guilds, and commission counts; formats 1–2
  readable. Validation and migration without reward replay.
- Race/class lore and stat previews at creation; a level-1 signature combat
  ability for all 13 classes, once per encounter; location-aware encounters.
- Five cities, five guilds (all joinable), 10 storyline quests, 20 side
  quests, 20 repeatable commissions — 50 playable quests with level,
  membership, and specific-guild prerequisites.
- Side Quests and Commissions boards; quest journal; commissions need explicit
  reacceptance and a fresh objective per run; side quests never repeat;
  payouts and completions guarded against duplicates.
- Quest handling in `src/world/` split into guild_quest, quest, sidequests,
  commissions. 14/14 tests on Linux.

## v2.0.0 — Refactoring milestone (retrospective)

**Period:** 2026-07-09 through 2026-10-30

Reorganized the single-file game into modules and improved build portability.
Retrospective label — no v2.0.0 tag was published at the time.

- [Initial refactor (`be6d543`)](https://github.com/asffxx/RPGGame/commit/be6d543)
  split `src/main.cpp` into core, data, entities, combat, systems, UI,
  application, and world modules.
- [Directory restructuring (`db9e14d`)](https://github.com/asffxx/RPGGame/commit/db9e14d)
  refined the source layout.
- [Display separation (`17e6ada`)](https://github.com/asffxx/RPGGame/commit/17e6ada)
  centralized gameplay output in display helpers.
- [Character-creation refactor (`e34675d`)](https://github.com/asffxx/RPGGame/commit/e34675d)
  organized creation and display code.
- [Portable screen clearing (`08d3538`)](https://github.com/asffxx/RPGGame/commit/08d3538)
  and [build/tooling update (`0125cdf`)](https://github.com/asffxx/RPGGame/commit/0125cdf)
  improved terminal behavior, added the current CMake setup, and refreshed the
  README.

## v1.1.0 — Early interface and travel fixes

**Date:** 2026-01-14

Named in [`0e2c157` — Update v1.1.0](https://github.com/asffxx/RPGGame/commit/0e2c157).

- Consistent screen clearing around travel and combat; shortened introduction.
- Fixed marked-location travel not updating location and type.

## v1.0.0 — Initial project (retrospective)

**Date:** 2026-01-14

Preserved on the [`v1` branch](https://github.com/asffxx/RPGGame/tree/v1),
whose tip is [`19b50cb`](https://github.com/asffxx/RPGGame/commit/19b50cb);
`main` carries current development.

- [Initial commit (`9c33a81`)](https://github.com/asffxx/RPGGame/commit/9c33a81):
  README, license, Git attributes.
- [Game commit (`b231a88`)](https://github.com/asffxx/RPGGame/commit/b231a88):
  the playable game in one `src/main.cpp` — creation, combat, exploration,
  inventory, progression, party, establishments.

The label groups both commits; the initial commit alone had no game source.

## Versioning

`v3.0.0` establishes the release baseline using `vMAJOR.MINOR.PATCH` tags; the
v1/v2 labels above are development eras, not release tags. Save-format numbers
are separate from game versions. SemVer rules apply: MAJOR for incompatible
interface or save changes, MINOR for features, PATCH for fixes.
