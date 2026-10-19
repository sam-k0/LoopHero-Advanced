# Loop Hero Advanced

A collection of quality of life improvements to the game

# Features planned
- Additional Language support
- Gear drop filter (automatically lock filtered gear types on drop)
- Sorting supplies

# Installation
Requires [YYToolkit](https://github.com/sam-k0/YYToolkit_LoopHero/releases/latest) and [LoopHeroCallbackCore](https://github.com/sam-k0/LoopHeroCallbackCore/releases/latest).

1. Load `LHAdvanced.dll` as a YYToolkit plugin.
2. Copy the contents of `resources/` next to `Loop Hero.exe`.

The whole mod can be turned off in `options.ini` (next to `Loop Hero.exe`):
```ini
[Advanced]
Enabled=0
```

# Features

## Additional Language support
Adds Polish, Thai and Vietnamese to the language selection in the options menu. Each language gets its own flag
button next to the original ones.

- Translations: `local/lang_pol.ini`, `local/lang_tha.ini`, `local/lang_vie.ini`
- Flags: `Advanced/flag_pol.png`, `Advanced/flag_tha.png`, `Advanced/flag_vie.png`

## Gear drop filter
Automatically pins (locks) fresh drops of the gear types you choose, the same as right-clicking them in the
inventory (the item is locked and moved to the front of the inventory).

- During an expedition, a filter button sits in the free top right cell of the equipment grid. Click it to open the
  filter menu.
- The menu has a checkbox for every gear type your class can currently drop.
- Only new drops are pinned
- The selection is saved in `options.ini` under `[PinFilter]`.

## Sorting supplies
The sort button in the camp's supply menu normally only sorts the supplies in storage. With this mod it also sorts
the equipped supplies of each type.
