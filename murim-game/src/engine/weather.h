/*
 * weather.h — Dynamic Weather and Wind Engine (4D Feel)
 * Heavenly Demon: Murim Chronicles
 */
#ifndef WEATHER_H
#define WEATHER_H

#include "types.h"

/* Initialize the weather system */
void weather_init(World *world);

/* Update weather transitions and wind physics */
void weather_update(Game *game, float dt);

/* Force a specific weather type */
void weather_set(World *world, WeatherType type);

/* Apply wind forces to a particle based on current wind */
void weather_apply_wind_to_particle(const World *world, Particle *p, float dt);

/* Draw weather overlays (rain, snow, fog, lightning) */
void weather_draw_overlay(const Game *game);

#endif /* WEATHER_H */
