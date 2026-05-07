/*
 * dungeon.h — Dungeon Gates & Instanced Dungeons (Solo Leveling)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef DUNGEON_H
#define DUNGEON_H

#include "../engine/types.h"

void dungeon_init(Game *game);
void dungeon_spawn_gates(Game *game);
void dungeon_update(Game *game, float dt);
void dungeon_draw_gates(const Game *game);
void dungeon_draw_floor(const Game *game);
void dungeon_enter(Game *game, int gate_idx);
void dungeon_exit(Game *game);
int  dungeon_find_nearby_gate(const Game *game, float range);
void dungeon_generate_floor(DungeonGate *gate, int rank, unsigned int seed);

#endif /* DUNGEON_H */
