/*
 * npc.h — NPC AI and entity management
 */
#ifndef NPC_H
#define NPC_H

#include "../engine/types.h"

/* Spawn the player entity at a position */
int npc_spawn_player(Game *game, Vec2 pos);

/* Spawn an NPC entity */
int npc_spawn(Game *game, EntityType type, Vec2 pos, const char *name);

/* Spawn enemies around a position */
void npc_populate_area(Game *game, Vec2 center, float radius, int count);

/* Update all NPC AI */
void npc_update_ai(Game *game, float dt);

/* Check if player is near any friendly NPC for interaction */
int npc_find_nearby_friendly(Game *game, float range);

#endif /* NPC_H */
