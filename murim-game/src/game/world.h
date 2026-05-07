/*
 * world.h — World generation and management
 */
#ifndef WORLD_H
#define WORLD_H

#include "../engine/types.h"

/* Generate the entire world procedurally */
void world_generate(World *world, unsigned int seed);

/* Generate a single chunk */
void world_generate_chunk(Chunk *chunk, int cx, int cy, unsigned int seed);

/* Check if a tile is walkable */
bool world_tile_walkable(const World *world, int tx, int ty);

/* Check if a position (pixel coords) is walkable */
bool world_pos_walkable(const World *world, float x, float y);

/* Get tile type at pixel position */
TileType world_get_tile_at(const World *world, float x, float y);

/* Update world systems (day/night) */
void world_update(World *world, float dt);

#endif /* WORLD_H */
