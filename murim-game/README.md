# ⚔️ Heavenly Demon: Murim Chronicles

A 2D open-world murim/anime-inspired game featuring martial arts cultivation, 
real-time combat, and procedural world generation.

Built with **C** (Raylib engine), **C++** game systems, and **Python** tooling.

---

## 🎮 Features

- **Open World** — Procedurally generated tile-based world with forests, mountains, rivers, deserts, and sect compounds
- **Cultivation System** — Progress through 7 realms: Mortal → Qi Gathering → Foundation → Core Formation → Nascent Soul → Spirit Severing → Transcendence
- **Martial Arts Combat** — Real-time melee combat with 5 unique qi techniques (Sword Slash, Qi Blast, Heavenly Strike, Dragon Fist, Sword Rain)
- **NPC AI** — Enemies that wander, detect, chase, and attack. Friendly NPCs with dialogue
- **Day/Night Cycle** — Dynamic lighting with dawn/dusk warm tints
- **Particle Effects** — Qi auras, combat hits, breakthrough explosions
- **Minimap** — Real-time minimap showing terrain and entity positions
- **HUD** — Health/QI bars, cultivation progress, technique display, stats screen

## 🏗️ Tech Stack

| Language | Role |
|----------|------|
| **C** (Raylib 5.5) | Core engine: rendering, input, camera, particles |
| **C** | Game systems: combat, cultivation, world, NPC AI |
| **Python 3** | World generation tools, data pipeline |
| **CMake** | Build system |

## 🚀 Quick Start

### Prerequisites

```bash
# macOS
brew install raylib cmake

# Linux (Ubuntu/Debian)
sudo apt install libraylib-dev cmake build-essential
```

### Build & Run

```bash
# Configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .

# Run
./bin/murim_game
```

### Python World Generator

```bash
# Generate world data with visualization
python3 tools/world_gen.py --seed 42 --size 256 --visualize

# Custom output directory
python3 tools/world_gen.py --output assets/data
```

## 🎯 Controls

| Key | Action |
|-----|--------|
| **WASD** / Arrow Keys | Move |
| **SPACE** | Attack (melee) |
| **Q** | Use Qi Technique |
| **E** | Cultivate / Interact with NPC |
| **TAB** | Stats & Techniques Screen |
| **1-5** | Switch Active Technique |
| **ESC** | Pause Menu |
| **R** | Restart (from pause/game over) |
| **F3** | Toggle Debug Info |
| **Mouse Wheel** | Zoom In/Out |

## 📁 Project Structure

```
murim-game/
├── CMakeLists.txt           # Build system
├── src/
│   ├── main.c               # Game loop & input
│   ├── engine/
│   │   ├── types.h          # All type definitions
│   │   ├── renderer.c/.h    # Tile, entity, particle rendering
│   │   ├── camera.c/.h      # Smooth-follow camera + shake
│   │   └── particles.c/.h   # Particle system (qi, combat)
│   ├── game/
│   │   ├── world.c/.h       # Procedural world generation
│   │   ├── combat.c/.h      # Combat & techniques
│   │   ├── cultivation.c/.h # Cultivation realm system
│   │   └── npc.c/.h         # NPC AI & spawning
│   └── ui/
│       └── hud.c/.h         # HUD, menus, dialogue
├── tools/
│   └── world_gen.py         # Python world generator
└── assets/
    └── data/                # Generated world data (JSON)
```

## 🗡️ Cultivation Realms

| Realm | HP | QI | ATK | DEF |
|-------|----|----|-----|-----|
| Mortal | 100 | 20 | 8 | 3 |
| Qi Gathering | 150 | 60 | 15 | 6 |
| Foundation | 250 | 120 | 25 | 12 |
| Core Formation | 400 | 200 | 40 | 20 |
| Nascent Soul | 600 | 350 | 65 | 35 |
| Spirit Severing | 900 | 500 | 100 | 55 |
| Transcendence | 1500 | 800 | 160 | 90 |

## 📜 License

This project is for educational and personal use.
