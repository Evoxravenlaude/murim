/*
 * particles.c — Particle system implementation
 */
#include "particles.h"
#include <math.h>
#include <stdlib.h>

static int find_free_particle(Game *game)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!game->particles[i].active)
            return i;
    }
    return -1; /* no free slot */
}

void particle_spawn(Game *game, Vec2 pos, Vec2 vel, Color color,
                    float life, float size)
{
    int idx = find_free_particle(game);
    if (idx < 0) return;

    Particle *p = &game->particles[idx];
    p->active = true;
    p->pos = pos;
    p->vel = vel;
    p->color = color;
    p->life = life;
    p->max_life = life;
    p->size = size;
    p->rotation = 0;
    p->rot_speed = ((float)(rand() % 100) / 50.0f - 1.0f) * 3.0f;
}

void particle_burst(Game *game, Vec2 pos, Color color, int count,
                    float speed, float life, float size)
{
    for (int i = 0; i < count; i++) {
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        float spd = speed * (0.5f + (float)(rand() % 100) / 100.0f);
        Vec2 vel = {
            cosf(angle) * spd,
            sinf(angle) * spd
        };
        float sz = size * (0.5f + (float)(rand() % 100) / 200.0f);
        particle_spawn(game, pos, vel, color, life, sz);
    }
}

void particle_qi_effect(Game *game, Vec2 pos, CultivationRealm realm)
{
    /* Realm-specific colors */
    static const Color REALM_PARTICLE_COLORS[] = {
        { 200, 200, 200, 200 }, /* mortal - white */
        { 120, 200, 255, 200 }, /* qi gathering - light blue */
        { 100, 255, 150, 200 }, /* foundation - green */
        { 255, 215, 0, 200 },   /* core formation - gold */
        { 200, 100, 255, 200 }, /* nascent soul - purple */
        { 255, 80, 80, 200 },   /* spirit severing - red */
        { 255, 255, 255, 255 }, /* transcendence - pure white */
    };

    Color color = REALM_PARTICLE_COLORS[realm];
    int count = 3 + (int)realm * 2;

    for (int i = 0; i < count; i++) {
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        float dist = 15.0f + (float)(rand() % 20);
        Vec2 spawn = {
            pos.x + cosf(angle) * dist,
            pos.y + sinf(angle) * dist
        };
        Vec2 vel = {
            (pos.x - spawn.x) * 0.5f,
            (pos.y - spawn.y) * 0.5f - 20.0f
        };
        particle_spawn(game, spawn, vel, color, 0.8f + (float)(rand() % 50) / 100.0f,
                       2.0f + realm * 0.5f);
    }
}

void particle_hit_effect(Game *game, Vec2 pos)
{
    particle_burst(game, pos, (Color){255, 100, 50, 230}, 8, 80.0f, 0.4f, 3.0f);
    particle_burst(game, pos, (Color){255, 255, 200, 200}, 4, 40.0f, 0.3f, 2.0f);
}

void particles_update(Game *game, float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &game->particles[i];
        if (!p->active) continue;

        p->life -= dt;
        if (p->life <= 0) {
            p->active = false;
            continue;
        }

        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;
        p->vel.y += 30.0f * dt; /* gravity */
        p->vel.x *= 0.98f;      /* drag */
        p->rotation += p->rot_speed * dt;
    }
}
