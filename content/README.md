# Data files

Canonical content for the game lives here as JSON. The terminal game reads
these files through `src/gameplay/datafile.cpp`; the pixel game will read the
same files with its engine's JSON parser. Nothing in this directory is
compiled into the binary.

## Format

A strict JSON subset:

- objects, arrays, strings, numbers, booleans, and `null`
- no comments, no trailing commas, no duplicate keys
- numbers use plain JSON grammar; `01`, `.5`, `1.`, and `Infinity` are errors
- strings may use the standard escapes and `\uXXXX` sequences, including
  surrogate pairs
- files are UTF-8 and at most 4 MB

Errors name the file, line, and column, for example:

    data/recipes.json: line 12, column 5: expected ':'

## Identifiers

Every record has an `id`:

- lowercase snake_case: letters, digits, and underscores
- starts with a letter, does not end with an underscore
- unique within its file

`src/core/ids.cpp` derives ids from display names for the existing C++
catalogs, so `Iron Sword` becomes `iron_sword`. Duplicate names get `_2`, `_3`
in catalog order. New data files state ids explicitly.

Records reference each other by id, never by array position:

- a recipe input names an item id
- a dialogue condition names a flag, faction, quest, or item id
- a quest reward names an item id

An unknown reference is a load error. It is never skipped or defaulted.

## Files

| File | Status | Holds |
| --- | --- | --- |
| `materials.json` | Implemented, Phase 3 | Materials, weights, sell values, drops, commission rewards |
| `recipes.json` | Phase 3 | Recipes: inputs, output, station, unlock |
| `quests.json` | Phase 4 | Data-authored quests and item rewards |
| `npcs/*.json` | Phase 4 | Fixed-location special NPCs and dialogue assignment |
| `dialogue/*.json` | Phase 4 | Dialogue nodes, conditions, effects |
| `story/acts.json` | Phase 5 | Acts, beats, completion conditions |
| `factions.json` | Phase 6 | Guild factions and standing thresholds |
| `export/*.json` | Phase 8 | Existing catalogs exported with the same ids |

Until a phase lands, its file does not exist. Do not add a stub: a loader that
accepts an empty catalog hides a missing export.

## Runtime loading

`RPG_DATA_DIR` explicitly selects a directory. Otherwise the game prefers `data/`
in its working directory, then the build-time source directory. Both catalogs
must load successfully before the title screen. They are cached until restart.
Schema errors identify the file and field; only syntax errors include positions.

## Material schema

Root: `{ "materials": [...] }`. Each record requires `id`, `name`, `description`,
`weight` (positive, at most 1000), and `sellValue` (integer copper, 0..100000000).
Optional `source` describes where it comes from; optional `maxStack` defaults
to 999 and accepts 1..1000000.

Optional `drops` lists `{ "enemy": "bonewalker", "chance": 0.25,
"minQuantity": 1, "maxQuantity": 2 }`. Enemy ids must exist, chance must be in
(0,1], and quantities satisfy 1 <= min <= max <= 1000. Each rule rolls separately.

Optional `questRewards` lists `{ "quest": "commission_supplies", "quantity": 3 }`.
Quest ids must exist; quantities must fit the material stack limit. All material
rewards must fit before any quest reward or completion is committed.

## Recipe schema

Root: `{ "recipes": [...] }`. Each record requires `id`, `name`, `station`,
`inputs`, `output`, and `unlock`. Station is `forge`, `alchemy`, or `kitchen`.
Inputs are a nonempty array of `{ "item": "iron_ore", "quantity": 3 }`;
output has the same shape. Quantities are integers 1..1000. Inputs cannot repeat
an id. Items resolve across materials and the compiled equipment, potion, and
food catalogs.

Unlocks are `{ "type": "always" }` or `{ "type": "guild", "id": "ironward" }`,
`{ "type": "quest", "id": "side_waystone" }`, or
`{ "type": "flag", "id": "smith_mastery" }`. Guild unlocks require membership;
quest unlocks require claimed completion; flag unlocks require a nonzero value.
Already-known recipes remain available. Flag ids are syntax-checked, since
dialogue/story definitions arrive in later phases.

## Reference rules

- Items: ids from `src/catalogs/equipment.cpp`, `potion.cpp`, and `food.cpp`.
- Enemies: ids from `src/catalogs/enemy_data.cpp`.
- Locations: ids from `src/catalogs/location.cpp`.
- Quests: ids from `src/world/sidequests.cpp`, `commissions.cpp`, and
  `guild_quest.cpp`.
- Factions: guild ids from `src/catalogs/guild.cpp`.

`tests/data_tests.cpp` proves every catalog id is valid and unique. A rename
that changes an id breaks saves on purpose; add a migration instead.
