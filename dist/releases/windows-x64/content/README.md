# Game content

This folder contains the people, places, items, and stories used by RPGGame.

- **characters/** — races, classes, abilities, and city residents.
- **items/** — equipment, food, potions, and spells.
- **crafting/** — materials and recipes.
- **narrative/** — guilds, quests, rumors, conversations, and reputation rules.
- **story/** — the story board's acts and beats.
- **world/** — locations, enemies, events, and harmful effects.

Keep this folder with the game when downloading or moving it.

## Making changes

Follow the examples in the existing files and restart the game after editing.
Keep internal names marked `id` unchanged so saved games still recognize them.
If a file is missing or contains an error, the game may stop and explain what
needs fixing.

## Reputation (v3.7.0)

`narrative/reputation.json` defines five factions, tier thresholds, shop price
modifiers, stock gates, and guild quest standing rewards. City standing derives
from its guild. Values clamp to -100..100 and never decay.

Resident tier lines live in `characters/dialogue_npcs.json`; decision effects
live in `narrative/dialogues/`. Helpful choices give +3, dismissive choices -4.
Five side jobs use `minimumStanding: 6`; other jobs default to no minimum.
Friendly stock is hidden below Friendly; hostile-hidden stock disappears only
at Hostile. Shops outside guild cities retain normal stock and prices.
