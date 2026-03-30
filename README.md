# MageQuest

A JRPG inspired by Chrono Trigger and Final Fantasy for the [PocketMage PDA](https://github.com/ashtf8/PocketMage_PDA).

Turn-based combat with field encounters, party management, and Tech special attacks via the capacitive touch slider.

## Design Philosophy

- **1-bit pixel art** optimized for E-Ink — high contrast sprites, dithered backgrounds, woodcut aesthetic
- **Dual-display gameplay** — E-Ink for battle scenes and exploration map, OLED for real-time status and menus
- **Turn-based** — each action triggers one E-Ink refresh, perfect for the display technology
- **Touch slider integration** — the PocketMage's capacitive strip triggers devastating Tech attacks

## Features

- **Field exploration** with top-down tile map (20×15, 16px tiles)
- **On-field enemies** (no random encounters — Chrono Trigger style)
- **Turn-based battle system** with Attack / Magic / Item / Defend
- **Party of 2**: Arion (Mage — Fire, Heal) and Kael (Warrior — Slash, Guard)
- **Tech system**: Touch slider triggers area damage special attack
- **Enemy types**: Slimes, Goblins (more planned)
- **1-bit sprite system** using PROGMEM bitmap arrays

## Controls

### Exploration
| Key | Action |
|-----|--------|
| W/A/S/D | Move |
| Enter | Interact / Engage enemy |

### Battle
| Key | Action |
|-----|--------|
| W/S | Navigate menu |
| A | Back (submenu) |
| Enter | Confirm action |
| Touch Slider / E | Tech special attack (area damage) |

## Installation

OTA app for PocketMage V3:

1. Copy `src/MageQuest.cpp` to your PlatformIO project's `src/` folder
2. Set the `OTA_APP` build flag
3. Compile and flash via the PocketMage App Loader

## Roadmap

- [ ] More enemy types (Imps, Dragons, Bosses)
- [ ] Equipment system (weapons, armor)
- [ ] Multiple maps / dungeons
- [ ] NPC dialogue system
- [ ] Save/load game state to SD card
- [ ] Boss battles with unique mechanics
- [ ] Experience points and level progression
- [ ] More party members
- [ ] Animated battle sprites (frame cycling between E-Ink refreshes)

## Architecture

```
MageQuest.cpp
├── Sprites (PROGMEM 16x16 1-bit bitmaps)
├── Map System (tile-based, PROGMEM)
├── Field Enemies (on-map encounters)
├── Battle Engine
│   ├── Turn management
│   ├── Action system (Attack/Magic/Item/Defend/Tech)
│   ├── Damage calculation
│   └── Enemy AI
├── Drawing: Explore (tile renderer + camera)
├── Drawing: Battle (scene + UI + damage numbers)
└── OTA Entry Points (APP_INIT / processKB_APP / einkHandler_APP)
```

## License

Apache-2.0

## Credits

- Game by [AEB-DevLabs](https://github.com/AEB-DevLabs)
- Built for the [PocketMage PDA](https://pocketmage.org) by [Ashtf](https://github.com/ashtf8)
