/*
 * weather.c — Dynamic Weather and Wind Engine (4D Feel)
 * Heavenly Demon: Murim Chronicles
 */
#include "weather.h"
#include "particles.h"
#include "camera.h"
#include <stdlib.h>
#include <math.h>

void weather_init(World *world)
{
    world->weather.current = WEATHER_CLEAR;
    world->weather.target = WEATHER_CLEAR;
    world->weather.transition = 1.0f;
    world->weather.intensity = 0.0f;
    world->weather.change_timer = 60.0f; /* Change weather every 60s for demo */
    world->weather.lightning_timer = 0;
    world->weather.lightning_flash = false;
    world->weather.wind_strength = 0.2f;
    world->weather.wind_angle = 0.0f;
}

void weather_set(World *world, WeatherType type)
{
    world->weather.target = type;
    world->weather.transition = 0.0f;
    world->weather.change_timer = 120.0f;
}

void weather_update(Game *game, float dt)
{
    World *w = &game->world;
    WeatherState *ws = &w->weather;

    /* Weather transitions */
    if (ws->transition < 1.0f) {
        ws->transition += dt * 0.1f; /* 10 second transition */
        if (ws->transition >= 1.0f) {
            ws->transition = 1.0f;
            ws->current = ws->target;
        }
    }

    /* Change timer */
    ws->change_timer -= dt;
    if (ws->change_timer <= 0) {
        int r = rand() % 100;
        if (r < 40) weather_set(w, WEATHER_CLEAR);
        else if (r < 60) weather_set(w, WEATHER_CLOUDY);
        else if (r < 80) weather_set(w, WEATHER_RAIN);
        else if (r < 90) weather_set(w, WEATHER_STORM);
        else weather_set(w, WEATHER_FOG);
    }

    /* Wind updates — Guiding Wind (§3 GDD) */
    /* Blend natural drift with direction toward nearest quest objective / gate */
    float natural_drift = ws->wind_angle + (float)((rand() % 100) - 50) / 100.0f;
    float guide_angle = natural_drift; /* default: drift naturally */
    {
        const Entity *p = &game->entities[game->player_id];
        float best_dist = 999999.0f;
        bool found = false;

        /* Point toward nearest active dungeon gate */
        for (int i = 0; i < MAX_GATES; i++) {
            const DungeonGate *g = &game->gates[i];
            if (!g->active || g->is_cleared) continue;
            float dx = g->world_pos.x - p->pos.x;
            float dy = g->world_pos.y - p->pos.y;
            float d = dx*dx + dy*dy;
            if (d < best_dist) { best_dist = d; guide_angle = atan2f(dy, dx); found = true; }
        }
        /* Mix: 70% guide, 30% natural when objective exists */
        if (found) {
            /* Normalize angle difference */
            float diff = guide_angle - ws->wind_angle;
            while (diff > 3.14159f) diff -= 6.28318f;
            while (diff < -3.14159f) diff += 6.28318f;
            ws->wind_angle += diff * dt * 0.3f; /* Gentle steer toward objective */
        } else {
            ws->wind_angle += (natural_drift - ws->wind_angle) * dt * 0.1f;
        }
    }
    
    float target_wind_str = 0.2f;
    if (ws->current == WEATHER_STORM || ws->target == WEATHER_STORM) target_wind_str = 0.8f;
    else if (ws->current == WEATHER_RAIN || ws->target == WEATHER_RAIN) target_wind_str = 0.5f;
    ws->wind_strength += (target_wind_str - ws->wind_strength) * dt * 0.5f;

    /* Lightning in storms */
    ws->lightning_flash = false;
    if (ws->current == WEATHER_STORM) {
        ws->lightning_timer -= dt;
        if (ws->lightning_timer <= 0) {
            ws->lightning_flash = true;
            ws->lightning_timer = 2.0f + (float)(rand() % 50) / 10.0f;
            camera_shake(game, 15.0f, 0.4f); /* Thunder shake */
        }
    }

    /* Spawn weather particles (Screen space / local to camera) */
    if (ws->current == WEATHER_RAIN || ws->current == WEATHER_STORM || ws->transition < 1.0f) {
        float rain_intensity = (ws->current == WEATHER_STORM) ? 1.0f : 0.5f;
        if (ws->current != WEATHER_RAIN && ws->current != WEATHER_STORM) rain_intensity = 0.0f;
        
        /* Blend intensity */
        float actual_intensity = rain_intensity * ws->transition;
        if (ws->transition < 1.0f && (ws->target == WEATHER_RAIN || ws->target == WEATHER_STORM)) {
             actual_intensity = ((ws->target == WEATHER_STORM) ? 1.0f : 0.5f) * ws->transition;
        }

        int drops = (int)(actual_intensity * 20.0f);
        for (int i = 0; i < drops; i++) {
            /* Spawn rain slightly above camera view */
            Vec2 p = {
                game->camera.target.x + (rand() % (SCREEN_WIDTH + 400)) - (SCREEN_WIDTH/2 + 200),
                game->camera.target.y - SCREEN_HEIGHT/2 - 50 + (rand() % 100)
            };
            Color c = { 150, 200, 255, 150 };
            particle_burst(game, p, c, 1, 0, 0.5f, 1.0f); /* We'll hijack particles for rain */
            /* Force rain velocity downward + wind */
            for (int j = 0; j < MAX_PARTICLES; j++) {
                if (game->particles[j].active && game->particles[j].life == 0.5f) {
                    game->particles[j].vel.x = cosf(ws->wind_angle) * ws->wind_strength * 1000.0f;
                    game->particles[j].vel.y = 800.0f; /* Fast falling */
                    game->particles[j].size = 1.0f;
                    game->particles[j].has_trail = true;
                    break;
                }
            }
        }
    }
}

void weather_apply_wind_to_particle(const World *world, Particle *p, float dt)
{
    const WeatherState *ws = &world->weather;
    p->vel.x += cosf(ws->wind_angle) * ws->wind_strength * 200.0f * dt;
    p->vel.y += sinf(ws->wind_angle) * ws->wind_strength * 200.0f * dt;
}

void weather_draw_overlay(const Game *game)
{
    const WeatherState *ws = &game->world.weather;
    
    /* Lightning flash */
    if (ws->lightning_flash) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 255, 255, 255, 200 });
    }

    /* Fog */
    if (ws->current == WEATHER_FOG || ws->target == WEATHER_FOG) {
        float fog = (ws->current == WEATHER_FOG) ? 1.0f : 0.0f;
        if (ws->transition < 1.0f) {
            fog = (ws->target == WEATHER_FOG) ? ws->transition : 1.0f - ws->transition;
        }
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 200, 210, 220, (unsigned char)(150 * fog) });
    }

    /* Storm darkening */
    if (ws->current == WEATHER_STORM || ws->target == WEATHER_STORM) {
        float storm = (ws->current == WEATHER_STORM) ? 1.0f : 0.0f;
        if (ws->transition < 1.0f) {
            storm = (ws->target == WEATHER_STORM) ? ws->transition : 1.0f - ws->transition;
        }
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 10, 20, 40, (unsigned char)(100 * storm) });
    }
}
