#!/usr/bin/env python3
"""
world_gen.py — Python World Generation Tool
Heavenly Demon: Murim Chronicles

Generates world data files (JSON) that can be loaded by the game engine.
This demonstrates the Python tooling component of the multi-language architecture.

Features:
- Procedural terrain generation using Perlin-like noise
- Sect placement with buildings and NPCs
- Quest and dialogue data generation
- World statistics and visualization

Usage:
    python3 tools/world_gen.py [--seed SEED] [--size SIZE] [--output DIR] [--visualize]
"""

import json
import math
import random
import argparse
import os
import sys
from dataclasses import dataclass, field, asdict
from typing import List, Dict, Tuple, Optional

# ─── Constants ───────────────────────────────────────────
TILE_GRASS = 0
TILE_GRASS_DARK = 1
TILE_DIRT = 2
TILE_STONE = 3
TILE_WATER = 4
TILE_SAND = 5
TILE_TREE = 6
TILE_MOUNTAIN = 7
TILE_WALL = 8
TILE_FLOOR_WOOD = 9
TILE_FLOOR_STONE = 10
TILE_PATH = 11
TILE_BRIDGE = 12
TILE_FLOWER_RED = 13
TILE_FLOWER_BLUE = 14
TILE_BAMBOO = 15
TILE_SECT_GATE = 16
TILE_SECT_FLOOR = 17

TILE_NAMES = {
    TILE_GRASS: "grass", TILE_GRASS_DARK: "dark_grass", TILE_DIRT: "dirt",
    TILE_STONE: "stone", TILE_WATER: "water", TILE_SAND: "sand",
    TILE_TREE: "tree", TILE_MOUNTAIN: "mountain", TILE_WALL: "wall",
    TILE_FLOOR_WOOD: "wood_floor", TILE_FLOOR_STONE: "stone_floor",
    TILE_PATH: "path", TILE_BRIDGE: "bridge", TILE_FLOWER_RED: "red_flower",
    TILE_FLOWER_BLUE: "blue_flower", TILE_BAMBOO: "bamboo",
    TILE_SECT_GATE: "sect_gate", TILE_SECT_FLOOR: "sect_floor"
}

CULTIVATION_REALMS = [
    "Mortal", "Qi Gathering", "Foundation Establishment",
    "Core Formation", "Nascent Soul", "Spirit Severing", "Transcendence"
]


# ─── Noise Generation ───────────────────────────────────
class NoiseGenerator:
    """Simple value noise generator for terrain."""

    def __init__(self, seed: int):
        self.seed = seed

    def _hash(self, x: int, y: int, seed_offset: int = 0) -> float:
        """Hash-based pseudo-random for 2D coordinates."""
        h = self.seed + seed_offset
        h ^= x * 374761393
        h ^= y * 668265263
        h = ((h ^ (h >> 13)) * 1274126177) & 0x7FFFFFFF
        h = (h ^ (h >> 16)) & 0x7FFFFFFF
        return h / 0x7FFFFFFF

    def smooth_noise(self, x: float, y: float, seed_offset: int = 0) -> float:
        """Smoothly interpolated noise."""
        ix, iy = int(math.floor(x)), int(math.floor(y))
        fx, fy = x - ix, y - iy

        # Smooth interpolation
        fx = fx * fx * (3 - 2 * fx)
        fy = fy * fy * (3 - 2 * fy)

        a = self._hash(ix, iy, seed_offset)
        b = self._hash(ix + 1, iy, seed_offset)
        c = self._hash(ix, iy + 1, seed_offset)
        d = self._hash(ix + 1, iy + 1, seed_offset)

        ab = a + (b - a) * fx
        cd = c + (d - c) * fx
        return ab + (cd - ab) * fy

    def fractal_noise(self, x: float, y: float, octaves: int = 5,
                      seed_offset: int = 0) -> float:
        """Multi-octave fractal noise."""
        value = 0.0
        amplitude = 1.0
        frequency = 1.0
        max_val = 0.0

        for i in range(octaves):
            value += self.smooth_noise(x * frequency, y * frequency,
                                       seed_offset + i * 1000) * amplitude
            max_val += amplitude
            amplitude *= 0.5
            frequency *= 2.0

        return value / max_val


# ─── Data Classes ────────────────────────────────────────
@dataclass
class NPCData:
    name: str
    type: str  # "friendly", "hostile", "merchant", "elder", "beast"
    x: float
    y: float
    realm: str
    realm_level: int
    dialogue: List[str] = field(default_factory=list)

@dataclass
class SectData:
    name: str
    x: int
    y: int
    width: int
    height: int
    rank: str  # "minor", "major", "supreme"
    elder_name: str
    npcs: List[NPCData] = field(default_factory=list)

@dataclass
class QuestData:
    id: str
    title: str
    description: str
    giver_name: str
    objective: str
    reward_gold: int
    reward_xp: int
    min_realm: str

@dataclass
class WorldData:
    seed: int
    width: int
    height: int
    tiles: List[List[int]] = field(default_factory=list)
    sects: List[SectData] = field(default_factory=list)
    npcs: List[NPCData] = field(default_factory=list)
    quests: List[QuestData] = field(default_factory=list)
    spawn_x: float = 0
    spawn_y: float = 0


# ─── Sect Name Generator ────────────────────────────────
SECT_PREFIXES = [
    "Heavenly", "Azure", "Crimson", "Golden", "Shadow", "Jade",
    "Iron", "Thunder", "Dragon", "Phoenix", "Mystic", "Eternal",
    "Divine", "Sacred", "Dark", "Celestial", "Immortal", "Storm"
]
SECT_SUFFIXES = [
    "Sword Sect", "Fist School", "Palm Pavilion", "Peak Academy",
    "Valley Sect", "Mountain Sect", "Cloud Sect", "Flame Order",
    "Wind Hall", "Moon Palace", "Star Sect", "Tiger Clan"
]

NPC_SURNAMES = [
    "Chen", "Li", "Wang", "Zhang", "Liu", "Yang", "Huang", "Zhao",
    "Wu", "Zhou", "Lin", "Xu", "Sun", "Ma", "Gu", "Song"
]
NPC_GIVEN_NAMES = [
    "Wei", "Feng", "Long", "Tian", "Yun", "Ming", "Xiao", "Jun",
    "Hao", "Lei", "Shi", "Shan", "Qing", "Zhen", "Bai", "Rui"
]

BEAST_NAMES = [
    "Spirit Wolf", "Iron Boar", "Flame Fox", "Jade Serpent",
    "Thunder Hawk", "Stone Bear", "Shadow Cat", "Wind Tiger",
    "Blood Bat", "Storm Eagle", "Frost Lizard", "Earth Worm"
]


def generate_npc_name() -> str:
    return f"{random.choice(NPC_SURNAMES)} {random.choice(NPC_GIVEN_NAMES)}"

def generate_sect_name() -> str:
    return f"{random.choice(SECT_PREFIXES)} {random.choice(SECT_SUFFIXES)}"


# ─── World Generator ────────────────────────────────────
class WorldGenerator:
    def __init__(self, seed: int, width: int = 256, height: int = 256):
        self.seed = seed
        self.width = width
        self.height = height
        self.noise = NoiseGenerator(seed)
        self.world = WorldData(seed=seed, width=width, height=height)
        random.seed(seed)

    def generate(self) -> WorldData:
        """Generate the complete world."""
        print(f"🌍 Generating world (seed: {self.seed}, size: {self.width}x{self.height})...")

        print("  [1/5] Generating terrain...")
        self._generate_terrain()

        print("  [2/5] Placing sects...")
        self._place_sects()

        print("  [3/5] Generating paths...")
        self._generate_paths()

        print("  [4/5] Populating NPCs...")
        self._populate_npcs()

        print("  [5/5] Creating quests...")
        self._generate_quests()

        self._find_spawn()
        self._print_stats()

        return self.world

    def _generate_terrain(self):
        """Generate terrain tiles using fractal noise."""
        self.world.tiles = []
        for y in range(self.height):
            row = []
            for x in range(self.width):
                nx = x / self.width
                ny = y / self.height

                elevation = self.noise.fractal_noise(nx * 8, ny * 8, 5, 0)
                moisture = self.noise.fractal_noise(nx * 6 + 100, ny * 6 + 100, 4, 1000)
                detail = self.noise.fractal_noise(nx * 20, ny * 20, 3, 2000)

                # Island shaping
                dx, dy = nx - 0.5, ny - 0.5
                dist = math.sqrt(dx*dx + dy*dy) * 2
                elevation -= dist * 0.4

                tile = self._determine_tile(elevation, moisture, detail, x, y)
                row.append(tile)
            self.world.tiles.append(row)

    def _determine_tile(self, elev: float, moist: float, detail: float,
                        x: int, y: int) -> int:
        if elev < 0.28: return TILE_WATER
        if elev < 0.32: return TILE_SAND
        if elev > 0.78: return TILE_MOUNTAIN
        if elev > 0.70: return TILE_STONE

        if moist > 0.6 and 0.35 < elev < 0.65:
            if detail > 0.7: return TILE_TREE
            if detail > 0.65: return TILE_BAMBOO
            return TILE_GRASS_DARK

        if 0.35 < elev < 0.55:
            if detail > 0.85: return TILE_FLOWER_RED
            if detail > 0.82: return TILE_FLOWER_BLUE
            if moist < 0.35: return TILE_DIRT
            return TILE_GRASS

        if detail > 0.75 and moist > 0.4: return TILE_TREE
        if self.noise._hash(x, y, 999) > 0.8: return TILE_GRASS_DARK
        return TILE_GRASS

    def _place_sects(self):
        """Place sect compounds throughout the world."""
        num_sects = max(3, self.width // 50)
        attempts = 0
        max_attempts = num_sects * 20

        while len(self.world.sects) < num_sects and attempts < max_attempts:
            attempts += 1
            sx = random.randint(30, self.width - 40)
            sy = random.randint(30, self.height - 40)

            # Check if area is suitable (mostly grass/dirt)
            suitable = True
            for dy in range(-5, 15):
                for dx in range(-5, 15):
                    tx, ty = sx + dx, sy + dy
                    if 0 <= tx < self.width and 0 <= ty < self.height:
                        if self.world.tiles[ty][tx] in (TILE_WATER, TILE_MOUNTAIN):
                            suitable = False
                            break
                if not suitable:
                    break

            # Check distance from other sects
            for s in self.world.sects:
                if abs(s.x - sx) < 30 and abs(s.y - sy) < 30:
                    suitable = False
                    break

            if not suitable:
                continue

            # Place sect
            size = random.choice([8, 10, 12])
            rank = random.choice(["minor", "major", "supreme"])
            elder_name = f"Grand Elder {generate_npc_name()}"

            sect = SectData(
                name=generate_sect_name(),
                x=sx, y=sy, width=size, height=size,
                rank=rank, elder_name=elder_name
            )

            # Write tiles
            for dy in range(size):
                for dx in range(size):
                    tx, ty = sx + dx, sy + dy
                    if 0 <= tx < self.width and 0 <= ty < self.height:
                        if dx == 0 or dx == size-1 or dy == 0 or dy == size-1:
                            self.world.tiles[ty][tx] = TILE_WALL
                        else:
                            self.world.tiles[ty][tx] = TILE_SECT_FLOOR

            # Gate
            gate_x = sx + size // 2
            gate_y = sy + size - 1
            if 0 <= gate_x < self.width and 0 <= gate_y < self.height:
                self.world.tiles[gate_y][gate_x] = TILE_SECT_GATE

            self.world.sects.append(sect)

    def _generate_paths(self):
        """Generate paths connecting sects."""
        if len(self.world.sects) < 2:
            return

        for i in range(len(self.world.sects) - 1):
            s1 = self.world.sects[i]
            s2 = self.world.sects[i + 1]

            # Simple L-shaped path
            x, y = s1.x + s1.width // 2, s1.y + s1.height
            tx, ty = s2.x + s2.width // 2, s2.y

            while x != tx:
                if 0 <= x < self.width and 0 <= y < self.height:
                    if self.world.tiles[y][x] not in (TILE_WALL, TILE_SECT_GATE,
                                                        TILE_SECT_FLOOR, TILE_WATER):
                        self.world.tiles[y][x] = TILE_PATH
                x += 1 if tx > x else -1

            while y != ty:
                if 0 <= x < self.width and 0 <= y < self.height:
                    if self.world.tiles[y][x] not in (TILE_WALL, TILE_SECT_GATE,
                                                        TILE_SECT_FLOOR, TILE_WATER):
                        self.world.tiles[y][x] = TILE_PATH
                y += 1 if ty > y else -1

    def _populate_npcs(self):
        """Populate the world with NPCs."""
        # Add sect NPCs
        for sect in self.world.sects:
            # Elder
            self.world.npcs.append(NPCData(
                name=sect.elder_name, type="elder",
                x=sect.x + sect.width / 2, y=sect.y + sect.height / 2,
                realm=random.choice(CULTIVATION_REALMS[3:]),
                realm_level=random.randint(5, 9),
                dialogue=[
                    f"Welcome to the {sect.name}.",
                    "Train hard, and you shall reach the pinnacle.",
                    "Our sect has stood for a thousand years."
                ]
            ))

            # Sect disciples
            for _ in range(3):
                self.world.npcs.append(NPCData(
                    name=generate_npc_name(), type="friendly",
                    x=sect.x + random.randint(2, sect.width - 2),
                    y=sect.y + random.randint(2, sect.height - 2),
                    realm=random.choice(CULTIVATION_REALMS[:3]),
                    realm_level=random.randint(1, 5),
                    dialogue=[f"I'm training at the {sect.name}!"]
                ))

        # Wild NPCs and beasts
        num_wild = self.width * self.height // 500
        for _ in range(num_wild):
            x = random.randint(5, self.width - 5)
            y = random.randint(5, self.height - 5)

            if self.world.tiles[y][x] in (TILE_WATER, TILE_MOUNTAIN, TILE_WALL):
                continue

            r = random.random()
            if r < 0.3:
                self.world.npcs.append(NPCData(
                    name=generate_npc_name(), type="hostile",
                    x=x, y=y,
                    realm=random.choice(CULTIVATION_REALMS[:4]),
                    realm_level=random.randint(1, 7)
                ))
            elif r < 0.5:
                self.world.npcs.append(NPCData(
                    name=random.choice(BEAST_NAMES), type="beast",
                    x=x, y=y,
                    realm=random.choice(CULTIVATION_REALMS[:3]),
                    realm_level=random.randint(1, 5)
                ))
            elif r < 0.7:
                self.world.npcs.append(NPCData(
                    name=generate_npc_name(), type="merchant",
                    x=x, y=y,
                    realm=random.choice(CULTIVATION_REALMS[1:3]),
                    realm_level=random.randint(3, 7),
                    dialogue=["I have wares for sale!", "Spirit stones, fresh today!"]
                ))

    def _generate_quests(self):
        """Generate quest data."""
        quest_templates = [
            ("Bandit Slayer", "Defeat {n} rogue cultivators in the wild",
             "defeat_enemies", 50, 100, "Mortal"),
            ("Beast Hunter", "Slay {n} spirit beasts terrorizing travelers",
             "defeat_beasts", 80, 150, "Qi Gathering"),
            ("Herb Gathering", "Collect {n} spirit herbs from the bamboo grove",
             "collect_herbs", 30, 50, "Mortal"),
            ("Sect Defender", "Protect the sect from {n} invaders",
             "defend_sect", 120, 250, "Foundation Establishment"),
            ("Secret Realm", "Find and explore the hidden cave in the mountains",
             "explore", 200, 400, "Core Formation"),
        ]

        for i, (title, desc, obj, gold, xp, realm) in enumerate(quest_templates):
            n = random.randint(3, 8)
            giver = random.choice(self.world.npcs).name if self.world.npcs else "Unknown"
            self.world.quests.append(QuestData(
                id=f"quest_{i:03d}",
                title=title,
                description=desc.format(n=n),
                giver_name=giver,
                objective=f"{obj}:{n}",
                reward_gold=gold,
                reward_xp=xp,
                min_realm=realm
            ))

    def _find_spawn(self):
        """Find a valid spawn point near center."""
        cx, cy = self.width // 2, self.height // 2
        for r in range(50):
            for dx in range(-r, r+1):
                for dy in range(-r, r+1):
                    x, y = cx + dx, cy + dy
                    if 0 <= x < self.width and 0 <= y < self.height:
                        if self.world.tiles[y][x] in (TILE_GRASS, TILE_PATH,
                                                        TILE_GRASS_DARK):
                            self.world.spawn_x = x * 32 + 16
                            self.world.spawn_y = y * 32 + 16
                            return

    def _print_stats(self):
        """Print world generation statistics."""
        tile_counts = {}
        for row in self.world.tiles:
            for tile in row:
                name = TILE_NAMES.get(tile, "unknown")
                tile_counts[name] = tile_counts.get(name, 0) + 1

        total = self.width * self.height
        print(f"\n📊 World Statistics:")
        print(f"  Size: {self.width}x{self.height} ({total:,} tiles)")
        print(f"  Sects: {len(self.world.sects)}")
        print(f"  NPCs: {len(self.world.npcs)}")
        print(f"  Quests: {len(self.world.quests)}")
        print(f"  Spawn: ({self.world.spawn_x}, {self.world.spawn_y})")
        print(f"\n  Tile Distribution:")
        for name, count in sorted(tile_counts.items(), key=lambda x: -x[1]):
            pct = count / total * 100
            bar = "█" * int(pct / 2)
            print(f"    {name:15s}: {count:6d} ({pct:5.1f}%) {bar}")

    def save(self, output_dir: str):
        """Save world data to JSON files."""
        os.makedirs(output_dir, exist_ok=True)

        # Save world config (without full tilemap for readability)
        config = {
            "seed": self.world.seed,
            "width": self.world.width,
            "height": self.world.height,
            "spawn_x": self.world.spawn_x,
            "spawn_y": self.world.spawn_y,
        }
        with open(os.path.join(output_dir, "world_config.json"), "w") as f:
            json.dump(config, f, indent=2)

        # Save sects
        sects_data = [asdict(s) for s in self.world.sects]
        with open(os.path.join(output_dir, "sects.json"), "w") as f:
            json.dump(sects_data, f, indent=2)

        # Save NPCs
        npcs_data = [asdict(n) for n in self.world.npcs]
        with open(os.path.join(output_dir, "npcs.json"), "w") as f:
            json.dump(npcs_data, f, indent=2)

        # Save quests
        quests_data = [asdict(q) for q in self.world.quests]
        with open(os.path.join(output_dir, "quests.json"), "w") as f:
            json.dump(quests_data, f, indent=2)

        # Save tilemap (compressed — rows as strings)
        tilemap = []
        for row in self.world.tiles:
            tilemap.append("".join(f"{t:02x}" for t in row))
        with open(os.path.join(output_dir, "tilemap.json"), "w") as f:
            json.dump(tilemap, f)

        print(f"\n💾 World saved to {output_dir}/")

    def visualize_ascii(self):
        """Print ASCII visualization of the world."""
        TILE_CHARS = {
            TILE_GRASS: ".", TILE_GRASS_DARK: ",", TILE_DIRT: ":",
            TILE_STONE: "#", TILE_WATER: "~", TILE_SAND: "o",
            TILE_TREE: "T", TILE_MOUNTAIN: "^", TILE_WALL: "█",
            TILE_FLOOR_WOOD: "=", TILE_FLOOR_STONE: "-",
            TILE_PATH: "·", TILE_BRIDGE: "=",
            TILE_FLOWER_RED: "*", TILE_FLOWER_BLUE: "+",
            TILE_BAMBOO: "|", TILE_SECT_GATE: "G", TILE_SECT_FLOOR: "_"
        }

        print(f"\n🗺️  World Map (1 char = 1 tile, showing every 4th tile):")
        step = max(1, self.width // 80)
        for y in range(0, self.height, step):
            line = ""
            for x in range(0, self.width, step):
                tile = self.world.tiles[y][x]
                line += TILE_CHARS.get(tile, "?")
            print(f"  {line}")


# ─── Main ────────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser(
        description="Heavenly Demon: Murim Chronicles — World Generator"
    )
    parser.add_argument("--seed", type=int, default=None,
                        help="Random seed (default: random)")
    parser.add_argument("--size", type=int, default=256,
                        help="World size in tiles (default: 256)")
    parser.add_argument("--output", type=str, default="assets/data",
                        help="Output directory for generated data")
    parser.add_argument("--visualize", action="store_true",
                        help="Show ASCII visualization of generated world")

    args = parser.parse_args()

    seed = args.seed if args.seed is not None else random.randint(0, 999999)
    gen = WorldGenerator(seed, args.size, args.size)
    gen.generate()
    gen.save(args.output)

    if args.visualize:
        gen.visualize_ascii()

    print("\n✅ World generation complete!")


if __name__ == "__main__":
    main()
