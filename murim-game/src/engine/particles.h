/*
 * particles.h — Particle system for qi effects, combat visuals
 */
#ifndef PARTICLES_H
#define PARTICLES_H

#include "types.h"

/* Spawn a single particle */
void particle_spawn(Game *game, Vec2 pos, Vec2 vel, Color color,
                    float life, float size);

/* Spawn a burst of particles */
void particle_burst(Game *game, Vec2 pos, Color color, int count,
                    float speed, float life, float size);

/* Spawn qi cultivation particles */
void particle_qi_effect(Game *game, Vec2 pos, CultivationRealm realm);

/* Spawn hit/combat particles */
void particle_hit_effect(Game *game, Vec2 pos);

/* Update all particles */
void particles_update(Game *game, float dt);

#endif /* PARTICLES_H */
