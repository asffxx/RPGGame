# Game content

This folder contains the people, places, items, and stories used by RPGGame.

- **characters/** — races, classes, abilities, and city residents.
- **items/** — equipment, food, potions, spells, and equipment upgrade levels.
- **crafting/** — materials and recipes with station fees.
- **narrative/** — guilds, quests, rumors, conversations, and reputation rules.
- **story/** — the story board's acts and beats.
- **world/** — locations, enemies, events, and harmful effects.

Keep this folder with the game when downloading or moving it.

## Making changes

Follow the examples in the existing files and restart the game after editing.
Keep internal names marked `id` unchanged so saved games still recognize them.
If a file is missing or contains an error, the game may stop and explain what
needs fixing.

## Gap mechanics (v3.8.0)

`world/debuffs.json` is the single status catalog. Each record has a stable `id`,
a base `durationTurns`, stacking cap, refresh extension, per-stack damage/drain or
stat penalties, and a maximum-stack amplifier. All equipment, spells, and enemies
reference debuffs by display name or id.

`items/upgrades.json` defines three sequential equipment upgrade levels. Each
level lists required materials by item id and `feeCopper`. Upgrades add 15% of the
base stats per level. Recipes in `crafting/recipes.json` also carry a mandatory
`feeCopper` charged once per craft. Repairs are priced from the item's shop price.

## Reputation (v3.7.0)

`narrative/reputation.json` defines five factions, tier thresholds, shop price
modifiers, stock gates, and guild quest standing rewards. City standing derives
from its guild. Values clamp to -100..100 and never decay.

Resident tier lines live in `characters/dialogue_npcs.json`; decision effects
live in `narrative/dialogues/`. Helpful choices give +3, dismissive choices -4.
Five side jobs use `minimumStanding: 6`; other jobs default to no minimum.
Friendly stock is hidden below Friendly; hostile-hidden stock disappears only
at Hostile. Shops outside guild cities retain normal stock and prices.
