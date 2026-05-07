/*
 * environment.h — Temperature, Altitude, Oxygen, Hazards
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../engine/types.h"

void environment_init(Entity *player);
void environment_update(Game *game, float dt);
void environment_draw_overlay(const Game *game);
void environment_draw_hud(const Game *game);
EnvironmentZone environment_get_zone(const Game *game, Vec2 pos);

#endif /* ENVIRONMENT_H */
