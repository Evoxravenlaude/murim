/*
 * environment.h — Temperature, Altitude, Oxygen, Hazards
 * Heavenly Demon: Murim Chronicles v4.0
 */
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../engine/types.h"

void environment_init(Entity *player);
void environment_update(Game *game, float dt);
void environment_draw_overlay(const Game *game);
void environment_draw_hud(const Game *game);
EnvironmentZone environment_get_zone(const Game *game, Vec2 pos);
/* v4.0 survival interaction */
void environment_eat(Game *game);
void environment_drink(Game *game);

#endif /* ENVIRONMENT_H */
