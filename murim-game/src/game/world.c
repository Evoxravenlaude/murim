/*
 * world.c — Procedural world generation using simplex-like noise
 * Heavenly Demon: Murim Chronicles
 *
 * Generates a rich murim world with:
 * - Mountains, forests, plains, rivers, deserts
 * - Sect compounds with gates and buildings
 * - Roads connecting locations
 * - Bamboo groves, flower fields, and villages
 */
#include "world.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ─── Simple hash-based noise ─────────────────────────── */
static float hash2d(int x, int y, unsigned int seed)
{
    unsigned int h = seed;
    h ^= (unsigned int)x * 374761393u;
    h ^= (unsigned int)y * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h = h ^ (h >> 16);
    return (float)(h & 0x7FFFFFFF) / (float)0x7FFFFFFF;
}

static float smooth_noise(float x, float y, unsigned int seed)
{
    int ix = (int)floorf(x);
    int iy = (int)floorf(y);
    float fx = x - ix;
    float fy = y - iy;

    /* Smooth interpolation */
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);

    float a = hash2d(ix, iy, seed);
    float b = hash2d(ix + 1, iy, seed);
    float c = hash2d(ix, iy + 1, seed);
    float d = hash2d(ix + 1, iy + 1, seed);

    float ab = a + (b - a) * fx;
    float cd = c + (d - c) * fx;
    return ab + (cd - ab) * fy;
}

static float fractal_noise(float x, float y, unsigned int seed, int octaves)
{
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float max_val = 0.0f;

    for (int i = 0; i < octaves; i++) {
        value += smooth_noise(x * frequency, y * frequency, seed + i * 1000) * amplitude;
        max_val += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return value / max_val;
}

/* ─── Biome determination ─────────────────────────────── */
static TileType determine_tile(float elevation, float moisture, float detail,
                                int wx, int wy, unsigned int seed)
{
    /* Water */
    if (elevation < 0.28f) return TILE_WATER;

    /* Sand/beach near water */
    if (elevation < 0.32f) return TILE_SAND;

    /* Mountains */
    if (elevation > 0.78f) return TILE_MOUNTAIN;

    /* Stone at high elevation */
    if (elevation > 0.70f) return TILE_STONE;

    /* Forest */
    if (moisture > 0.6f && elevation > 0.35f && elevation < 0.65f) {
        if (detail > 0.7f) return TILE_TREE;
        if (detail > 0.65f) return TILE_BAMBOO;
        return TILE_GRASS_DARK;
    }

    /* Plains with flowers */
    if (elevation > 0.35f && elevation < 0.55f) {
        if (detail > 0.85f) return TILE_FLOWER_RED;
        if (detail > 0.82f) return TILE_FLOWER_BLUE;
        if (moisture < 0.35f) return TILE_DIRT;
        return TILE_GRASS;
    }

    /* Default grassland */
    if (detail > 0.75f && moisture > 0.4f) return TILE_TREE;
    if (hash2d(wx, wy, seed + 999) > 0.8f) return TILE_GRASS_DARK;
    return TILE_GRASS;
}

/* ─── Structure placement ─────────────────────────────── */

/* Place a sect compound at the given chunk-local coordinates */
static void place_sect(Chunk *chunk, int ox, int oy, int size)
{
    /* Outer walls */
    for (int y = oy; y < oy + size && y < CHUNK_SIZE; y++) {
        for (int x = ox; x < ox + size && x < CHUNK_SIZE; x++) {
            if (x < 0 || y < 0) continue;

            if (x == ox || x == ox + size - 1 || y == oy || y == oy + size - 1) {
                chunk->tiles[y][x] = TILE_WALL;
            } else {
                chunk->tiles[y][x] = TILE_SECT_FLOOR;
            }
        }
    }

    /* Gate entrance */
    int gate_x = ox + size / 2;
    int gate_y = oy + size - 1;
    if (gate_x >= 0 && gate_x < CHUNK_SIZE && gate_y >= 0 && gate_y < CHUNK_SIZE) {
        chunk->tiles[gate_y][gate_x] = TILE_SECT_GATE;
        if (gate_x - 1 >= 0) chunk->tiles[gate_y][gate_x - 1] = TILE_SECT_GATE;
    }
}

/* Place a path through the chunk */
static void place_path(Chunk *chunk, int cx, int cy, unsigned int seed)
{
    /* Horizontal path */
    if (hash2d(cx, cy, seed + 5000) > 0.6f) {
        int y = CHUNK_SIZE / 2 + (int)(hash2d(cx, cy, seed + 5001) * 4) - 2;
        for (int x = 0; x < CHUNK_SIZE; x++) {
            if (y >= 0 && y < CHUNK_SIZE &&
                chunk->tiles[y][x] != TILE_WATER &&
                chunk->tiles[y][x] != TILE_WALL &&
                chunk->tiles[y][x] != TILE_SECT_GATE) {
                chunk->tiles[y][x] = TILE_PATH;
                if (y + 1 < CHUNK_SIZE &&
                    chunk->tiles[y+1][x] != TILE_WATER)
                    chunk->tiles[y+1][x] = TILE_PATH;
            }
        }
    }

    /* Vertical path */
    if (hash2d(cx, cy, seed + 6000) > 0.6f) {
        int x = CHUNK_SIZE / 2 + (int)(hash2d(cx, cy, seed + 6001) * 4) - 2;
        for (int y = 0; y < CHUNK_SIZE; y++) {
            if (x >= 0 && x < CHUNK_SIZE &&
                chunk->tiles[y][x] != TILE_WATER &&
                chunk->tiles[y][x] != TILE_WALL &&
                chunk->tiles[y][x] != TILE_SECT_GATE) {
                chunk->tiles[y][x] = TILE_PATH;
                if (x + 1 < CHUNK_SIZE &&
                    chunk->tiles[y][x+1] != TILE_WATER)
                    chunk->tiles[y][x+1] = TILE_PATH;
            }
        }
    }
}

/* ─── Chunk generation ────────────────────────────────── */
void world_generate_chunk(Chunk *chunk, int cx, int cy, unsigned int seed)
{
    chunk->chunk_x = cx;
    chunk->chunk_y = cy;

    for (int ly = 0; ly < CHUNK_SIZE; ly++) {
        for (int lx = 0; lx < CHUNK_SIZE; lx++) {
            int wx = cx * CHUNK_SIZE + lx;
            int wy = cy * CHUNK_SIZE + ly;

            float nx = (float)wx / (float)WORLD_TILES_X;
            float ny = (float)wy / (float)WORLD_TILES_Y;

            float elevation = fractal_noise(nx * 8.0f, ny * 8.0f, seed, 5);
            float moisture  = fractal_noise(nx * 6.0f + 100.0f, ny * 6.0f + 100.0f,
                                            seed + 1, 4);
            float detail    = fractal_noise(nx * 20.0f, ny * 20.0f, seed + 2, 3);

            /* Island shaping — lower edges */
            float dx = nx - 0.5f;
            float dy = ny - 0.5f;
            float dist = sqrtf(dx * dx + dy * dy) * 2.0f;
            elevation -= dist * 0.4f;

            chunk->tiles[ly][lx] = determine_tile(elevation, moisture, detail,
                                                   wx, wy, seed);
        }
    }

    /* Place structures */
    float struct_chance = hash2d(cx, cy, seed + 3000);
    if (struct_chance > 0.85f) {
        /* Place a sect compound */
        place_sect(chunk, 3, 3, 10);
    }

    /* Place paths */
    place_path(chunk, cx, cy, seed);

    chunk->generated = true;
}

void world_generate(World *world, unsigned int seed)
{
    world->day_time = 8.0f;  /* Start at 8 AM */
    world->day_speed = 0.5f; /* 1 in-game hour = 2 real seconds */

    for (int cy = 0; cy < WORLD_CHUNKS_Y; cy++) {
        for (int cx = 0; cx < WORLD_CHUNKS_X; cx++) {
            world_generate_chunk(&world->chunks[cy][cx], cx, cy, seed);
        }
    }
}

bool world_tile_walkable(const World *world, int tx, int ty)
{
    if (tx < 0 || ty < 0 || tx >= WORLD_TILES_X || ty >= WORLD_TILES_Y)
        return false;

    int cx = tx / CHUNK_SIZE;
    int cy = ty / CHUNK_SIZE;
    int lx = tx % CHUNK_SIZE;
    int ly = ty % CHUNK_SIZE;

    TileType type = world->chunks[cy][cx].tiles[ly][lx];

    switch (type) {
    case TILE_WATER:
    case TILE_TREE:
    case TILE_MOUNTAIN:
    case TILE_WALL:
    case TILE_BAMBOO:
        return false;
    default:
        return true;
    }
}

bool world_pos_walkable(const World *world, float x, float y)
{
    /* Check a small box around the position for collision */
    int half = 6; /* character half-width */
    return world_tile_walkable(world, (int)((x - half) / TILE_SIZE), (int)((y - half) / TILE_SIZE)) &&
           world_tile_walkable(world, (int)((x + half) / TILE_SIZE), (int)((y - half) / TILE_SIZE)) &&
           world_tile_walkable(world, (int)((x - half) / TILE_SIZE), (int)((y + half) / TILE_SIZE)) &&
           world_tile_walkable(world, (int)((x + half) / TILE_SIZE), (int)((y + half) / TILE_SIZE));
}

TileType world_get_tile_at(const World *world, float x, float y)
{
    int tx = (int)(x / TILE_SIZE);
    int ty = (int)(y / TILE_SIZE);

    if (tx < 0 || ty < 0 || tx >= WORLD_TILES_X || ty >= WORLD_TILES_Y)
        return TILE_WATER;

    int cx = tx / CHUNK_SIZE;
    int cy = ty / CHUNK_SIZE;
    int lx = tx % CHUNK_SIZE;
    int ly = ty % CHUNK_SIZE;

    return world->chunks[cy][cx].tiles[ly][lx];
}

void world_update(World *world, float dt)
{
    world->day_time += world->day_speed * dt;
    if (world->day_time >= 24.0f)
        world->day_time -= 24.0f;
}
