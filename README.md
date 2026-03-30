# MageQuest — A Crystal Prophecy

A JRPG inspired by Chrono Trigger and Final Fantasy for the [PocketMage PDA](https://github.com/ashtf8/PocketMage_PDA).

> *"In an age long forgotten, five Crystals held the world in balance... but the Shadow Mage Volrath shattered the Crystal of Light."*

## Features

- **Title screen** with New Game / Continue menu
- **Storytelling intro** — animated text crawl with character portraits telling the Crystal Prophecy
- **Field exploration** — top-down tile map with trees, houses, signs, and NPCs with dialogue
- **On-field enemies** — Chrono Trigger style (no random encounters)
- **Turn-based battle** — Attack / Magic / Item / Defend menu system
- **Party of 2**: Arion (Mage: Flame, Heal) and Kael (Warrior: Cleave, Shield)
- **Tech system** — capacitive touch slider triggers devastating area-damage special attacks
- **Piezo audio** — menu beeps, attack SFX, magic chimes, victory fanfare, damage tones
- **NPC dialogue** — text boxes with character portraits and typewriter reveal
- **Screen transitions** — wipe effect between modes
- **Level progression** — XP, gold, stat growth on level up
- **1-bit pixel art** — anime-inspired sprites optimized for E-Ink display

## Controls

### Title Screen / Story
| Key | Action |
|-----|--------|
| W/S | Navigate menu |
| Enter | Confirm / Advance text |

### Exploration
| Key | Action |
|-----|--------|
| W/A/S/D | Move |
| Enter | Talk to NPC / Engage enemy |

### Battle
| Key | Action |
|-----|--------|
| W/S | Navigate menu |
| A | Back (submenu) |
| Enter | Confirm action |
| Touch Slider / E | Tech special attack (area damage!) |
| BKSP | Retry after game over |

## Story

Five Crystals once held the world in balance. The Shadow Mage Volrath shattered the Crystal of Light, plunging the realm into twilight. Two heroes answer the prophecy's call: **Arion**, a young mage gifted with elemental fire, and **Kael**, a knight whose blade never falters. Together they must cross the Verdant Fields, enter the Obsidian Tower, and restore the Crystal before darkness consumes all.

## Audio System

Uses the PocketMage piezo buzzer for:
- Menu navigation beeps (880Hz)
- Confirm tones (1200→1600Hz arpeggio)
- Attack hits (200Hz sawtooth)
- Magic casts (600→900→1200Hz sweep)
- Tech specials (300→600→1200→1800Hz cascade)
- Victory fanfare (C5→E5→G5→C6)
- Game over descent (400→350→300→200Hz)

## Architecture

```
MageQuest.cpp (OTA App)
├── Game States: TITLE → STORY → EXPLORE → BATTLE
├── Sprites (PROGMEM 16x16 & 32x48 1-bit bitmaps)
├── Audio Engine (piezo buzzer tone sequences)
├── Tile Map System (20x15, PROGMEM)
├── NPC Dialogue System (portraits + typewriter text)
├── Field Enemy System (on-map encounters)
├── Battle Engine
│   ├── Turn-based menu (Attack/Magic/Item/Defend)
│   ├── Tech system (capacitive slider trigger)
│   ├── Damage calculation with variance
│   ├── Enemy AI (random target selection)
│   └── Level up / XP / Gold
├── Screen Transitions (wipe effect)
├── Dual Display
│   ├── E-Ink: scenes, battles, dialogue
│   └── OLED: status, turn info, prompts
└── OTA Entry Points
```

## Installation

OTA app for PocketMage V3:

1. Copy `src/MageQuest.cpp` to your PlatformIO project
2. Set `OTA_APP` build flag
3. Flash via PocketMage App Loader

## Roadmap

- [ ] Obsidian Tower dungeon (second map area)
- [ ] Boss battle: Shadow Mage Volrath
- [ ] Equipment system (weapons, armor, accessories)
- [ ] Save/load to SD card
- [ ] More enemy types and boss mechanics
- [ ] Additional party members
- [ ] Shops and merchants
- [ ] Animated battle sprites
- [ ] Mini-map on OLED during exploration

## License

Apache-2.0

## Credits

- Game by [AEB-DevLabs](https://github.com/AEB-DevLabs)
- Built for [PocketMage PDA](https://pocketmage.org) by [Ashtf](https://github.com/ashtf8)
