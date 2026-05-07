/*
 * renderer.h — Rendering functions for tiles, entities, particles, and effects
 * Heavenly Demon: Murim Chronicles
 */
#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"

/* Initialize renderer resources */
void renderer_init(void);

/* Clean up renderer resources */
void renderer_cleanup(void);

/* Get color for a tile type */
Color tile_get_color(TileType type);

/* Draw visible world tiles */
void renderer_draw_world(const Game *game);

/* Draw a single entity with animation */
void renderer_draw_entity(const Entity *entity, float time);

/* Draw all active entities */
void renderer_draw_entities(const Game *game);

/* Draw all active particles */
void renderer_draw_particles(const Game *game);

/* Draw day/night overlay */
void renderer_draw_daynight(const Game *game);

/* Draw qi cultivation aura effect around entity */
void renderer_draw_qi_aura(Vec2 pos, CultivationRealm realm, float time);

/* Draw damage number floating up */
void renderer_draw_damage_number(Vec2 pos, int damage, float timer);

/* Draw a health bar above an entity */
void renderer_draw_health_bar(Vec2 pos, int hp, int max_hp, Color color);

#endif /* RENDERER_H */
